#ifndef WEBPKG_H
#define WEBPKG_H

#include <kcgi.h>
#include <sqlbox.h>

extern struct sqlbox *p;

enum key {
	KEY_QUERY,
	KEY__MAX
};

enum query {
	QUERY_SEARCH,
	QUERY_BY_FULLPKGPATH,
	QUERY__MAX
};

void		home_page(struct kreq*);
void		port_page(struct kreq*);
void		handle_err(struct kreq*, int);

#endif /* WEBPKG_H */
