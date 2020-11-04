create virtual table webpkg_fts using fts5(id,
                                           pkgstem,
                                           comment,
                                           descr_contents,
                                           maintainer);

insert into webpkg_fts
select pathid,
       pkgstem,
       comment,
       descr_contents,
       maintainer
  from portsq;

select webpkg_fts.pkgstem,
       webpkg_fts.comment,
       p.fullpkgname,
       paths.fullpkgpath
  from webpkg_fts
	 join _ports p on p.fullpkgpath = webpkg_fts.id
         join _paths paths on paths.id = webpkg_fts.id
 where webpkg_fts match 'godot';
