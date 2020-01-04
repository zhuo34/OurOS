#include "fs_cmd.h"

#include <driver/vga.h>

#include <ouros/fs/fs.h>
#include <ouros/error.h>

static char  pwd_buffer[PWD_BUF_SIZE];
static char* pwd_path = nullptr;

void pwd()
{
	kernel_printf("%s\n", pwd_path);
}

void ls(const char *dirname)
{
	// 打开目标文件
	if(!dirname || *dirname == '\0')
		dirname = pwd_path;
	if(kernel_strcmp(dirname, "~") == 0)
		dirname = HOME_PATH;
		
	file *dir = fs_open(dirname, F_MODE_READ);
	if(IS_ERR_PTR(dir)) {
		print_error_info(PTR_ERR(dir), dirname);
		return;
	}

	// 获得目标目录项
	dentry *entry = dir->f_op->list_dir(dir);
	if(IS_ERR_PTR(entry)) {
		print_error_info(PTR_ERR(entry), dirname);
		return;
	}

	// 列出目录
	list_sort(&entry->d_subdirs, sort_cmp);
	list_head *p;
	list_for_each(p, &entry->d_subdirs) {
		dentry *entry = container_of(p, dentry, d_sibling);
		int fgColor = entry->d_inode->i_isdir? VGA_YELLOW: VGA_BLUE;
		kernel_printf_color(fgColor, VGA_BLACK, "%s\n", entry->d_name.name);

		entry->d_pinned = false;
		entry->d_inode->i_pinned = false;
		d_dec_count(entry);
	}
	fs_close(dir);
}

void cd(const char *dirname)
{
	// 打开目标文件
	if(!dirname || *dirname == '\0' || kernel_strcmp(dirname, "~") == 0)
		dirname = HOME_PATH;
	
	// 更新dentry
	file *dir = fs_open(dirname, F_MODE_READ);
	if(IS_ERR_PTR(dir)) {
		print_error_info(PTR_ERR(dir), dirname);
		return;
	}
	set_pwd_dentry(dir->f_dentry);

	// 更新当前路径名
	pwd_path = pwd_buffer + PWD_BUF_SIZE;
	DEC_AND_PUT(pwd_path, '\0');

	dentry *entry = get_pwd_dentry();
	dentry *root = get_root_dentry();
	if(entry == root) {
		DEC_AND_PUT(pwd_path, '/');
		return;
	}

	while(entry != root) {
		int pos = entry->d_name.len - 1;
		for(; pos >= 0; pos--) {
			DEC_AND_PUT(pwd_path, entry->d_name.name[pos]);
		}
		DEC_AND_PUT(pwd_path, '/');

		entry = get_dentry_parent(entry);
	}
	fs_close(dir);
}

void cat(const char *filename)
{
	file *text_file = fs_open(filename, F_MODE_READ);
	if(IS_ERR_PTR(text_file)) {
		print_error_info(PTR_ERR(text_file), filename);
		return;
	}

	while(!eof(text_file)) {
		kernel_putchar(fs_getchar(text_file));
	}
	kernel_putchar('\n');
	fs_close(text_file);
}

void touch(const char *filename)
{
	file *fp = fs_open(filename, F_MODE_READ_WRITE);
	if(IS_ERR_PTR(fp)) {
		print_error_info(PTR_ERR(fp), filename);
		return;
	}
	fs_close(fp);
}

void mkdir(const char *filename)
{
	file *fp = fs_open(filename, F_MODE_READ_WRITE | F_MODE_DIR_MASK);
	if(IS_ERR_PTR(fp)) {
		print_error_info(PTR_ERR(fp), filename);
		return;
	}
	fs_close(fp);
}

void rm(const char *filename)
{
	int error = NO_ERROR;
	file *fp = fs_open(filename, F_MODE_READ);
	if(IS_ERR_PTR(fp)) {
		error = PTR_ERR(fp);
		goto exception;
	}
	error = fs_delete(fp);
	if(IS_ERR_VALUE(error)) {
		goto exception;
	}
	fs_close(fp);
	return;

exception:
	if(error == -ERROR_NOT_EMPTY) {
		kernel_printf_error("directory \"%s\" is not empty!\n", filename);
	} else {
		print_error_info(error, filename);
	}
}

void mv(const char *src, const char *dst)
{
	int error = NO_ERROR;
	file *src_file = fs_open(src, F_MODE_READ);
	if(IS_ERR_PTR(src_file)) {
		error = PTR_ERR(src_file);
		goto exception;
	}
	file *dst_file = fs_open(dst, F_MODE_WRITE);
	if(IS_ERR_PTR(dst_file)) {
		error = PTR_ERR(dst_file);
		goto exception;
	}

	while(!eof(src_file)) {
		error = fs_putchar(dst_file, fs_getchar(src_file));
		if(IS_ERR_VALUE(error)) {
			goto exception;
		}
	}
	error = fs_delete(src_file);
	if(IS_ERR_VALUE(error)) {
		goto exception;
	}
	fs_close(src_file);
	fs_close(dst_file);
	return;

exception:
	print_error_info(error, src);
}

void print_prompt()
{
	kernel_printf("[user@localhost ");
	char *path = pwd_path;
	if(kernel_strcmp(path, HOME_PATH) == 0) {
		path = "~";
	}
	kernel_printf("%s]$ ", path);
}

int sort_cmp(const struct list_head *a, const struct list_head *b)
{
	dentry *a_entry = container_of(a, dentry, d_sibling);
	dentry *b_entry = container_of(b, dentry, d_sibling);

	return a_entry->d_op->compare(a_entry->d_name, b_entry->d_name);
}

void print_error_info(int code, const char *path)
{
	if(code == -ERROR_NOT_DIR) {
		kernel_printf_error("\"%s\" is not a directory!\n", path);
	} else if(code == -ERROR_CACHE_FULL) {
		kernel_printf_error("fs cache full!\n");
	} else if(code == -ERROR_FILE_NOTFOUND) {
		kernel_printf_error("\"%s\" is not found!\n", path);
	} else {
		kernel_printf_error("error code: %d\n", code);
	}
}