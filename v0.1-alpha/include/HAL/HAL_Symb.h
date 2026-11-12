#ifndef AHS_H
#define AHS_H

struct hal_mod_t;
struct hal_dev_t;

typedef struct hal_mod_t {
	const char* id;
	int version;
	int (*open)(const struct hal_mod_t* module, const char* name, struct hal_dev_t** dev);
} hal_mod_t;

typedef struct hal_dev_t {
	struct hal_mod_t* common;
} hal_dev_t;

#endif
