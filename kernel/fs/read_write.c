#include "read_write.h"

#include <ouros/error.h>
#include <ouros/utils.h>
#include <ouros/fs/fscache.h>

#include <driver/vga.h>

int fs_read(file *fp, void *buffer, uint len)
{
	Byte* buf = (Byte*)buffer;
	int error = NO_ERROR;
	// 改变文件指针位置
	if(!(fp->f_mode & F_MODE_READ_MASK)) {
		error = -ERROR_ACCESS_AUTHORITY;
		goto exit;
	} else if(fp->f_pos + len > fp->f_size) {
		error = -ERROR_END_OF_FILE;
		goto exit;
	} else {
		fp->f_pos += len;
	}

	int cur_off = fp->f_cur_off;
	list_head *map_node = &fp->f_cur_map->map_node;
	// 读取文件内容
	int index = 0;
	while(true) {
		fs_map *map = container_of(map_node, fs_map, map_node);
		fspage *page = get_fspage(map_node, fp->f_dentry->d_sb->s_blocksize);
		if(cur_off + len <= page->p_pagesize) {
			// 在当前块结束
			kernel_memcpy(buf + index, page->p_data + cur_off, len);
			cur_off += len;

			fp->f_cur_off = cur_off;
			fp->f_cur_map = map;
			break;
		} else {
			// 将当前块读完并跳到下一块
			int delta = page->p_pagesize - cur_off;
			kernel_memcpy(buf + index, page->p_data + cur_off, delta);
			index += delta;

			map_node = map_node->next;
			len -= delta;
			cur_off = 0;
		}
	}

exit:
	return error;
}

int fs_write(file *fp, void *buffer, uint len)
{
	Byte* buf = (Byte*)buffer;
	int error = NO_ERROR;
	// 改变文件指针位置
	if(!(fp->f_mode & F_MODE_WRITE_MASK)) {
		error = -ERROR_ACCESS_AUTHORITY;
		goto exit;
	} else if(fp->f_pos + len > fp->f_size) {
		error = file_expand_to(fp, fp->f_pos + len);
		if(IS_ERR_VALUE(error))
			goto exit;
	}
	fp->f_pos += len;

	int cur_off = fp->f_cur_off;
	list_head *map_node = &fp->f_cur_map->map_node;
	// 覆盖文件指针处的内容
	int index = 0;
	while(true) {
		fs_map *map = container_of(map_node, fs_map, map_node);
		fspage *page = get_fspage(map_node, fp->f_dentry->d_sb->s_blocksize);
		if(cur_off + len <= page->p_pagesize) {
			// 在当前块结束
			kernel_memcpy(page->p_data + cur_off, buf + index, len);
			fp->f_dirt = true;
			page->p_dirt = true;
			cur_off += len;

			fp->f_cur_off = cur_off;
			fp->f_cur_map = map;
			break;
		} else {
			// 将当前块写完并跳到下一块
			int delta = page->p_pagesize - cur_off;
			kernel_memcpy(page->p_data + cur_off, buf + index, delta);
			fp->f_dirt = true;
			page->p_dirt = true;
			index += delta;

			map_node = map_node->next;
			len -= delta;
			cur_off = 0;
		}
	}

exit:
	return error;
}

int fs_seek(file *fp, int flag, int offset)
{
	// 根据参数移动文件当前指针
	if(flag == SEEK_HEAD) {
		fp->f_pos = 0;
	} else if(flag == SEEK_TAIL) {
		fp->f_pos = fp->f_size;
	}
	// 防止越界
	fp->f_pos += offset;
	if(fp->f_pos < 0) {
		fp->f_pos = 0;
	} else if(fp->f_pos > fp->f_size) {
		fp->f_pos = fp->f_size;
	}
	// 找到当前指向的块以及块内偏移量
	int blk_size = fp->f_dentry->d_sb->s_blocksize;
	int cnt = fp->f_pos;
	list_head *p;
	list_for_each(p, fp->f_data_map) {
		if(cnt <= blk_size)
			break;
		cnt -= blk_size;
	}
	fp->f_cur_map = container_of(p, fs_map, map_node);
	fp->f_cur_off = cnt;

	return NO_ERROR;
}

int fs_delete(file *fp)
{
	int error = NO_ERROR;

	// 获得文件对应的索引结点和父目录项
	inode *node = fp->f_dentry->d_inode;
	dentry *parent = get_dentry_parent(fp->f_dentry);
	dentry *curfile = fp->f_dentry;
	// 如果删除的是非空的文件夹，需要报错
	if(node->i_isdir) {
		dentry *entry = fp->f_op->list_dir(fp);
		if(IS_ERR_PTR(entry)) {
			error = PTR_ERR(entry);
			goto exit;
		}
		if(!list_empty(&entry->d_subdirs)) {
			error = -ERROR_NOT_EMPTY;
			goto exit;
		}
	}

	error = node->i_iop->delete(node, parent, curfile->d_name);
	if(IS_ERR_VALUE(error)) {
		goto exit;
	}

	// 防止fclose时改变外存
	fp->f_dirt = false;
exit:
	return error;
}

int file_expand_to(file *fp, int size)
{
	int error = NO_ERROR;
	if(size <= fp->f_size)
		goto exit;
	
	// 计算最后一块剩余的空间
	inode *node = fp->f_dentry->d_inode;
	super_block *sb = node->i_sb;
	int remain = file_last_blk_remain(fp);
	int expand = size - fp->f_size - remain;
	if(expand > 0) {
		// 扩展inode空间
		int expand_blks = ((expand + sb->s_blocksize - 1) & (~(sb->s_blocksize - 1)) ) >> sb->s_blockbits;
		error = node->i_iop->expand(node, expand_blks);
		if(IS_ERR_VALUE(error))
			goto exit;
	}
	fp->f_size = node->i_size = size;
exit:
	return error;
}

int fs_printf(file *fp, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);

	int error = fs_printf_argList(fp, format, argList);

	va_end(argList);
	return error;
}

int fs_printf_argList(file *fp, const char* format, va_list argList)
{
	int error = NO_ERROR;
	while (*format) {
		if (*format != '%') {
			error = fs_putchar(fp, *format++);
		} else {
			char type = *++format;
			if(type == 'c') {
				char valch = va_arg(argList, int);
				error = fs_putchar(fp, valch);
			} else if(type == 'd') {
				int valint = va_arg(argList, int);
				error = fs_putint(fp, valint);
			} else if(type == 'x' || type == 'X') {
				uint valint = va_arg(argList, uint);
				error = fs_puthex(fp, valint, type == 'X');
			} else if(type == 's') {
				char *valstr = va_arg(argList, char*);
				error = fs_puts(fp, valstr);
			} else {
				error = -ERROR_UNKNOWN_FORMAT;
			}
			format ++;
		}

		if(IS_ERR_VALUE(error)) {
			break;
		}
	}

	return error;
}

char fs_getchar(file *fp)
{
	char c;
	if(fs_read(fp, &c, 1) == EOF)
		c = EOF;
	return c;
}

int fs_putchar(file *fp, char ch)
{
	return fs_write(fp, &ch, 1);
}

int fs_puts(file *fp, const char *string) {
	int error = NO_ERROR;
	while (*string) {
		error = fs_putchar(fp, *string++);
		if(IS_ERR_VALUE(error))
			break;
	}
	return error;
}

int fs_putint(file *fp, int num) {
	// int 最大值为10位数，加上符号最多11位，再加上结尾'\0' 1位
	// 故字符串buffer取12位
	char buffer[12];
	buffer[11] = '\0';
	char *ptr = buffer + 11;

	bool isNegative = false;
	if (num < 0) {
		isNegative = true;
		num = -num;
	}
	
	int error = NO_ERROR;
	if (num == 0) {
		error = fs_putchar(fp, '0');
	} else {
		while (num) {
			ptr--;
			*ptr = (num % 10) + '0';
			num /= 10;
		}
		if (isNegative) {
			ptr--;
			*ptr = '-';
		}

		error = fs_puts(fp, ptr);
	}
	
	return error;
}

static const char *HEX_MAP = "0123456789abcdef";
int fs_puthex(file *fp, uint hex, bool isUpper) {
	char buffer[12];
	char *ptr = buffer + 11;
	buffer[11] = '\0';

	int error = NO_ERROR;
	if (hex == 0) {
		error = fs_putchar(fp, '0');
	} else {
		while (hex) {
			char value = hex & 0xF;
			ptr--;
			*ptr = HEX_MAP[value] + ((isUpper && value > 9)? ('A' - 'a'): 0);
			hex >>= 4;
		}
		error = fs_puts(fp, ptr);
	}
	return hex;
}

