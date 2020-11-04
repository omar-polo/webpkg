#include <sys/types.h>

#include <assert.h>
#include <err.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kcgi.h>
#include <kcgihtml.h>
#include <sqlbox.h>

#include "webpkg.h"

static void
search_form(struct kreq *r, struct khtmlreq *req)
{
	struct kpair *p;
	const char *query;

	if ((p = r->fieldmap[KEY_QUERY]))
		query = p->parsed.s;
	else
		query = NULL;

	khtml_attr(req, KELEM_FORM,
	    KATTR_METHOD, "GET",
	    KATTR_ACTION, "/",
	    KATTR__MAX);

	khtml_attr(req, KELEM_LABEL,
	    KATTR_FOR, "query",
	    KATTR__MAX);
	khtml_puts(req, "search for: ");
	khtml_closeelem(req, 1);
	khtml_attr(req, KELEM_INPUT,
	    KATTR_ID, "query",
	    KATTR_TYPE, "search",
	    KATTR_NAME, "query",
	    KATTR_VALUE, query == NULL ? "" : query,
	    KATTR_PLACEHOLDER, "query (e.g. \"fzf\")",
	    KATTR__MAX);

	khtml_closeelem(req, 1);
}

static void
init_page(struct kreq *r, struct khtmlreq *req, const char *title)
{
	khtml_elem(req, KELEM_DOCTYPE);
	khtml_attr(req, KELEM_HTML,
	    KATTR_LANG, "en", KATTR__MAX);
	khtml_elem(req, KELEM_HEAD);

	khtml_attr(req, KELEM_META,
	    KATTR_CHARSET, "utf-8", KATTR__MAX);

	khtml_attr(req, KELEM_META,
	    KATTR_NAME, "viewport",
	    KATTR_CONTENT, "initial-scale=1, width=device-width",
	    KATTR__MAX);

	khtml_elem(req, KELEM_TITLE);
	khtml_puts(req, "WebPKG");
	if (title) {
		khtml_puts(req, ": ");
		khtml_puts(req, title);
	}

	khtml_closeelem(req, 1);

	khtml_attr(req, KELEM_LINK,
	    KATTR_REL, "stylesheet",
	    KATTR_HREF, "/webpkg.css",
	    KATTR__MAX);
	khtml_closeelem(req, 1);

	khtml_elem(req, KELEM_BODY);

	khtml_elem(req, KELEM_HEADER);
	khtml_attr(req, KELEM_DIV,
	    KATTR_CLASS, "container", KATTR__MAX);
	khtml_elem(req, KELEM_H1);
	khtml_attr(req, KELEM_A,
	    KATTR_HREF, "/", KATTR__MAX);
	khtml_puts(req, "WebPKG");
	khtml_closeelem(req, 2); /* a, h1 */
	search_form(r, req);
	khtml_closeelem(req, 2); /* div, header */

	/* XXX: it should be MAIN, but khtml doesn't have it */
	khtml_attr(req, KELEM_DIV,
	    KATTR_CLASS, "main", KATTR__MAX);
}

static void
end_page(struct khtmlreq *req)
{
	khtml_closeelem(req, 1); /* div.main */

	khtml_elem(req, KELEM_FOOTER);
	khtml_attr(req, KELEM_DIV,
	    KATTR_CLASS, "container", KATTR__MAX);
	khtml_elem(req, KELEM_P);
	khtml_puts(req, "For any issue, please contact me at <op at omarpolo dot com>.");
	khtml_closeelem(req, 1);

	khtml_elem(req, KELEM_P);
	khtml_attr(req, KELEM_A,
	    KATTR_HREF, "https://git.omarpolo.com/webpkg",
	    KATTR_TARGET, "_blank",
	    KATTR_REL, "noopener",
	    KATTR__MAX);
	khtml_puts(req, "Source code");
	khtml_closeelem(req, 1);
	khtml_puts(req, ", ");
	khtml_attr(req, KELEM_A,
	    KATTR_HREF, "https://github.com/omar-polo/webpkg",
	    KATTR_TARGET, "_blank",
	    KATTR_REL, "noopener",
	    KATTR__MAX);
	khtml_puts(req, "github mirror");
	khtml_closeelem(req, 3); /* a, p, footer */
}

static void
do_search(const char *query, struct khtmlreq *req)
{
	struct sqlbox_parm parms = {
		.sparm	= query,
		.type	= SQLBOX_PARM_STRING,
		.sz	= 0,
	};
	const struct sqlbox_parmset *res;
	size_t stmtid = 0;
	int found_something = 0;

	if (strlen(query) < 3)
		goto end;

	stmtid = sqlbox_prepare_bind(p, 0, QUERY_SEARCH,
	    1, &parms, SQLBOX_STMT_MULTI);
	if (!stmtid)
		errx(1, "sqlbox_prepare_bind");

	khtml_elem(req, KELEM_DL);

	for (;;) {
		if ((res = sqlbox_step(p, stmtid)) == NULL)
			errx(1, "sqlbox_step");

		if (res->psz == 0 && res->code == SQLBOX_CODE_OK)
			break;

		found_something = 1;

		assert(res->psz == 4);
		assert(res->ps[0].type == SQLBOX_PARM_STRING);
		assert(res->ps[1].type == SQLBOX_PARM_STRING);
		assert(res->ps[2].type == SQLBOX_PARM_STRING);
		assert(res->ps[3].type == SQLBOX_PARM_STRING);

		khtml_elem(req, KELEM_DT);
		khtml_attr(req, KELEM_A,
		    KATTR_HREF, res->ps[3].sparm,
		    KATTR__MAX);
		khtml_puts(req, res->ps[0].sparm);
		khtml_closeelem(req, 1);
		khtml_puts(req, " ");
		khtml_elem(req, KELEM_CODE);
		khtml_puts(req, res->ps[2].sparm);
		khtml_closeelem(req, 2); /* code, dt */

		khtml_elem(req, KELEM_DD);
                khtml_puts(req, res->ps[1].sparm);
		khtml_closeelem(req, 1);
	}
	khtml_closeelem(req, 1);

end:
	if (!found_something) {
		khtml_elem(req, KELEM_P);
		khtml_elem(req, KELEM_EM);
		khtml_puts(req, "No results found");
		khtml_closeelem(req, 2); /* em, p */
	}

	if (stmtid && !sqlbox_finalise(p, stmtid))
		errx(1, "sqlbox_finalise");
}

void
home_page(struct kreq *r)
{
	struct kpair *p;
	struct khtmlreq req;

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[r->mime]);
	khttp_body(r);

	khtml_open(&req, r, 0);

	if ((p = r->fieldmap[KEY_QUERY])) {
		char *title;

		if (asprintf(&title, "search results for \"%s\"",
			    p->parsed.s) == -1)
			err(1, "asprintf");
		init_page(r, &req, title);
		free(title);

		khtml_attr(&req, KELEM_P,
		    KATTR_CLASS, "version-disclaimer",
		    KATTR__MAX);
		khtml_puts(&req, "Note that the version number shown may not be up-to-date with the actual version of the package in -CURRENT.");
		khtml_closeelem(&req, 1);
		do_search(p->parsed.s, &req);
	} else {
		init_page(r, &req, NULL);
		khtml_elem(&req, KELEM_P);
		khtml_puts(&req, "Try searching for something "
		    "using the form in the header.  What you type "
		    "will be matched against the package name (");
		khtml_elem(&req, KELEM_CODE);
		khtml_puts(&req, "pkgstem");
		khtml_closeelem(&req, 1);
		khtml_puts(&req, "), the comment, the DESCR "
			"and the maintainer.");
		khtml_closeelem(&req, 1);
	}

	end_page(&req);
	khtml_close(&req);
}

void
port_page(struct kreq *r)
{
	size_t stmtid;
	struct khtmlreq req;
	const struct sqlbox_parmset *res;
	struct sqlbox_parm parms = {
		.sparm	= r->fullpath+1,
		.type	= SQLBOX_PARM_STRING,
		.sz	= 0,
	};
	char *cvsweb;

	stmtid = sqlbox_prepare_bind(p, 0, QUERY_BY_FULLPKGPATH,
	    1, &parms, 0);
	if (!stmtid)
		errx(1, "sqlbox_prepare_bind");

	if ((res = sqlbox_step(p, stmtid)) == NULL)
		errx(1, "sqlbox_step");

	khtml_open(&req, r, 0);

	if (res->psz == 0 && res->code == SQLBOX_CODE_OK) {
		khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_404]);
		khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s",
		    kmimetypes[r->mime]);
		khttp_body(r);
		khtml_printf(&req, "%s not found", r->fullpath+1);
		goto end;
	}

	assert(res->psz == 7);
	assert(res->ps[0].type == SQLBOX_PARM_STRING);
	assert(res->ps[1].type == SQLBOX_PARM_STRING);
	assert(res->ps[2].type == SQLBOX_PARM_STRING);
	assert(res->ps[3].type == SQLBOX_PARM_STRING);
	assert(res->ps[4].type == SQLBOX_PARM_STRING);
	/* assert(res->ps[5].type == SQLBOX_PARM_STRING); can be null */
	/* assert(res->ps[6].type == SQLBOX_PARM_STRING); can be null */

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[KHTTP_200]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s", kmimetypes[r->mime]);
	khttp_body(r);

	init_page(r, &req, r->fullpath+1);

	khtml_elem(&req, KELEM_H2);
	khtml_puts(&req, res->ps[0].sparm); /* fullpkgpath */
	khtml_closeelem(&req, 1);

	khtml_attr(&req, KELEM_P,
	    KATTR_CLASS, "comment", KATTR__MAX);
	khtml_elem(&req, KELEM_EM);
	khtml_puts(&req, res->ps[2].sparm); /* comment */
	khtml_closeelem(&req, 2);

	khtml_elem(&req, KELEM_PRE);
	khtml_attr(&req, KELEM_CODE,
	    KATTR_CLASS, "pkg_add_sample", KATTR__MAX);
	khtml_printf(&req, "pkg_add %s", res->ps[1].sparm); /* pkgname */
	khtml_closeelem(&req, 2);

	khtml_elem(&req, KELEM_BLOCKQUOTE);
	khtml_puts(&req, res->ps[3].sparm); /* descr */
	khtml_closeelem(&req, 1);

	khtml_attr(&req, KELEM_P,
	    KATTR_CLASS, "maintainer", KATTR__MAX);
	khtml_printf(&req, "MAINTAINER: %s", res->ps[4].sparm); /* email */
	khtml_closeelem(&req, 1);

	if (asprintf(&cvsweb, "https://cvsweb.openbsd.org/ports/%s",
		    res->ps[0].sparm) == -1)
		err(1, "asprintf");

	khtml_attr(&req, KELEM_NAV,
	    KATTR_CLASS, "port-links", KATTR__MAX);
	khtml_elem(&req, KELEM_UL);
	khtml_elem(&req, KELEM_LI);
	khtml_attr(&req, KELEM_A,
	    KATTR_HREF, cvsweb,
	    KATTR__MAX);
	khtml_puts(&req, "CVS web");
	khtml_closeelem(&req, 2); /* a, li */

	if (res->ps[6].type != SQLBOX_PARM_NULL) {
		khtml_elem(&req, KELEM_LI);
		khtml_attr(&req, KELEM_A,
		    KATTR_HREF, res->ps[6].sparm,
		    KATTR__MAX);

		khtml_puts(&req, "WWW");
		khtml_closeelem(&req, 2); /* a, li */
	}
	khtml_closeelem(&req, 2); /* ul, nav */

	free(cvsweb);

	if (res->ps[5].type != SQLBOX_PARM_NULL) {
		khtml_elem(&req, KELEM_PRE);
		khtml_elem(&req, KELEM_CODE);
		khtml_puts(&req, res->ps[5].sparm); /* readme */
		khtml_closeelem(&req, 2);	    /* code, pre */
	}

	end_page(&req);

end:
	if (!sqlbox_finalise(p, stmtid))
		errx(1, "sqlbox_finalise");

	khtml_close(&req);
}

void
handle_err(struct kreq *r, int status)
{
	struct khtmlreq req;

	khttp_head(r, kresps[KRESP_STATUS], "%s", khttps[status]);
	khttp_head(r, kresps[KRESP_CONTENT_TYPE], "%s",
	    kmimetypes[KMIME_TEXT_PLAIN]);
	khttp_body(r);

	khtml_open(&req, r, 0);
	khtml_printf(&req, "%s\n", khttps[status]);
	khtml_close(&req);
}
