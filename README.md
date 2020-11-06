# WebPKG

WebPKG is a web interface for the OpenBSD package catalogue.  It
features a full-text search and packages pages.

## Building

##### On OpenBSD

	$ doas pkg_add kcgi sqlbox sqlports
	$ make install

This will install the webpkg executable, the sqlite3 database and the
CSS file in /var/www.  It’s recommended to use httpd with slowcgi:

	# rcctl enable httpd slowcgi
	# rcctl start httpd slowcgi

Then you need something along these lines in your `httpd.conf`

	server "webpkg.local" {
		listen on * port 80
		location found "/*" {
			pass
		}
		location "*" {
			fastcgi
			root "/cgi-bin/webpkg.cgi"
		}
	}

##### Other systems

The bad news is that you are (unfortunately) on your own.  The good
news is that deploying is simple. I’ve successfully ran webpkg
under FreeBSD in no time.

You need a webserver that is able to execute CGI scripts (or FastCGI,
but then you need slowcgi(8) or similar).  You also need to install
[kcgi](kcgi) and [sqlbox](sqlbox) and a POSIX make.

For the database, you can download the tarball of sqlports from your
preferred OpenBSD mirror ([for example](sqlports), and extract (it’s a
tar-bomb!) `share/sqlports`.

[kcgi]: https://kristaps.bsd.lv/kcgi/
[sqlbox]: https://kristaps.bsd.lv/sqlbox/
[sqlports]: https://cdn.openbsd.org/pub/OpenBSD/snapshots/packages/amd64/sqlports-7.32p0.tgz
