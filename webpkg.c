#include <sys/types.h>

#include <err.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kcgi.h>
#include <kcgihtml.h>
#include <sqlbox.h>

#include "webpkg.h"

#ifndef __OPENBSD__
# define pledge(a, b) 0
#endif

struct sqlbox *p;

const struct kvalid keys[KEY__MAX] = {
	{ kvalid_stringne, "query" }, /* KEY_QUERY */
};

enum page {
	PAGE_INDEX,
	PAGE__MAX
};

const size_t page_no_extra_permitted = 0;

const char *const pages[PAGE__MAX] = {
	"index",		/* PAGE_INDEX */
};

struct sqlbox_pstmt pstmts[QUERY__MAX] = {
	/* QUERY_SEARCH */
	{
		.stmt = ""
		"select webpkg_fts.pkgstem,"
		"       webpkg_fts.comment,"
		"       p.fullpkgname,"
		"       paths.fullpkgpath"
		"  from webpkg_fts"
		"         join _ports p on p.fullpkgpath = webpkg_fts.id"
		"         join _paths paths on paths.id = webpkg_fts.id"
		" where webpkg_fts match ?"
		" order by bm25(webpkg_fts)"
	},

	/* QUERY_BY_FULLPKGPATH */
	{
		.stmt = ""
		"select p.fullpkgpath,"
		"       pp.pkgstem,"
		"       pp.comment,"
		"       d.value,"
		"       replace(replace(e.value, '@', ' at '), '.', ' dot '),"
		"       r.value,"
		"       pp.homepage"
		"  from _paths p"
		"         join _descr d on d.fullpkgpath = p.id"
		"         join _ports pp on pp.fullpkgpath = p.id"
		"         join _email e on e.keyref = pp.maintainer"
		"         left join _readme r on r.fullpkgpath = p.id"
		" where p.fullpkgpath = ?"
	}
};

int
main(void)
{
	struct kreq r;
	struct sqlbox_cfg cfg;
	struct sqlbox_src srcs[] = {
		{.fname = "/webpkg.sqlite", .mode = SQLBOX_SRC_RO },
	};

	memset(&cfg, 0, sizeof(cfg));
	cfg.msg.func_short = warnx;
	cfg.srcs.srcs = srcs;
	cfg.srcs.srcsz = 1;
	cfg.stmts.stmts = pstmts;
	cfg.stmts.stmtsz = QUERY__MAX;

	if ((p = sqlbox_alloc(&cfg)) == NULL)
		errx(1, "sqlbox_alloc");
	if(!sqlbox_open_async(p, 0))
		errx(1, "sqlbox_open_async");

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	if (khttp_parse(&r, keys, KEY__MAX,
	    pages, PAGE__MAX, PAGE_INDEX) != KCGI_OK)
		return 0;

	if (r.mime != KMIME_TEXT_HTML)
		handle_err(&r, KHTTP_404);
	else if (r.method != KMETHOD_GET &&
	    r.method != KMETHOD_HEAD)
		handle_err(&r, KHTTP_405);
	else {
		switch (r.page) {
		case PAGE_INDEX:
			home_page(&r);
			break;

		case PAGE__MAX:
			port_page(&r);
			break;
		}
	}

	khttp_free(&r);
	sqlbox_free(p);
	return 0;
}
