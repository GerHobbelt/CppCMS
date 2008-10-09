pkgdata_DATA = look.opcode style.css adminstyle.css os.opcode
nobase_lib_LTLIBRARIES = libadmin_view.la
libadmin_view_la_SOURCES = admin_view.cpp

ADMIN_TEMPLATES=admin.tmpl admin_cache.tmpl admin_editcomment.tmpl admin_main.tmpl \
	admin_editoptions.tmpl  admin_login.tmpl  admin_sendtrackback.tmpl \
	admin_editcats.tmpl  admin_editlinks.tmpl  admin_editpost.tmpl setup_blog.tmpl

TEMPLATES= lnglst.tmpl error.tmpl main_page.tmpl page.tmpl post.tmpl\
	feed_comments.tmpl feed_posts.tmpl  sidebar.tmpl\
	trackback.tmpl 

look.opcode: ${TEMPLATES} master-def.tmpl
	tmpl_bld -o look.opcode -p messages.pot ${TEMPLATES} master-def.tmpl

os.opcode: ${TEMPLATES} master-os.tmpl
	tmpl_bld -o os.opcode -p messages-os.pot ${TEMPLATES} master-os.tmpl

admin_view.cpp: ${ADMIN_TEMPLATES}
	cppcms_tmpl_cc -d cppblog -o admin_view.cpp ${ADMIN_TEMPLATES}

clean-local:
	rm -f look.opcode look.s messages.pot os.opcode os.s messages-os.pot admin_view.cpp
