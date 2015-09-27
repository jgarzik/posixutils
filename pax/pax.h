#ifndef __PAX_INTERNAL_H__
#define __PAX_INTERNAL_H__

#include <stdint.h>
#include <string>

enum pax_error_codes {
	PXE_NONE		= 0,
	PXE_SYSERR		= -1,	/* see errno for more info */
	PXE_TRUNCATED		= -2,	/* unexpected EOF */
	PXE_GARBAGE		= -3,	/* invalid data received */
	PXE_MISC_ERR		= -4,	/* unspecified error; try to avoid */
};

struct pax_file_info {
	std::string		pathname;

	std::string		username;
	std::string		groupname;

	std::string		linkname;

	int			dev_major;
	int			dev_minor;

	dev_t			dev;
	ino_t			inode;
	mode_t			mode;
	nlink_t			nlink;
	uid_t			uid;
	gid_t			gid;
	dev_t			rdev;
	uintmax_t		size;
	time_t			atime;
	time_t			mtime;
	time_t			ctime;

	uintmax_t		compressed_size;

	bool			hardlink;
};

struct pax_operations {
	void *opaque;
	int (*output_actor)(void *opaque, const char *buf, size_t len);

	void (*input_init)(void);
	void (*input_fini)(void);
	int (*input)(const char *buf, size_t *buflen);

	int (*archive_start)(void);
	int (*archive_end)(void);

	int (*file_start)(struct pax_file_info *fi);
	int (*file_end)(struct pax_file_info *fi);
	int (*file_data)(struct pax_file_info *fi, const char *buf, size_t buflen);
};

/* pax.c */
extern void pax_init_operations(struct pax_operations *ops);

/* cpio.c */
extern void cpio_init_operations(struct pax_operations *ops);

/* ustar.c */
extern void ustar_blksz_check(void);
extern void ustar_init_operations(struct pax_operations *ops);

/* zip.c */
extern void zip_init_operations(struct pax_operations *ops);

/* main.c */
extern unsigned int block_size;
extern struct pax_operations pax_ops;
extern void pax_fi_clear(struct pax_file_info *fi);
extern long octal_str(const char *s, int len);

#endif /* __PAX_INTERNAL_H__ */
