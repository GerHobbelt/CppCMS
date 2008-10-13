pkgdata_DATA = style.css adminstyle.css
nobase_lib_LTLIBRARIES = libadmin_view.la libcontendend.la liborangesky.la
libadmin_view_la_SOURCES = admin_view.cpp
libcontendend_la_SOURCES = def.cpp
liborangesky_la_SOURCES = os.cpp


ADMIN_TEMPLATES=admin.tmpl admin_cache.tmpl admin_editcomment.tmpl admin_main.tmpl \
	admin_editoptions.tmpl  admin_login.tmpl  admin_sendtrackback.tmpl \
	admin_editcats.tmpl  admin_editlinks.tmpl  admin_editpost.tmpl setup_blog.tmpl

TEMPLATES= error.tmpl main_page.tmpl page.tmpl post.tmpl\
	feed_comments.tmpl feed_posts.tmpl \
	trackback.tmpl sidebar.tmpl 

def.cpp: ${TEMPLATES} master-def.tmpl master.tmpl
	cppcms_tmpl_cc -o def.cpp -d cppblog -n contendend master.tmpl master-def.tmpl ${TEMPLATES}

os.cpp: ${TEMPLATES} master-os.tmpl master.tmpl
	cppcms_tmpl_cc -o os.cpp -d cppblog -n orangesky master.tmpl master-os.tmpl ${TEMPLATES}

admin_view.cpp: ${ADMIN_TEMPLATES}
	cppcms_tmpl_cc -d cppblog -o admin_view.cpp ${ADMIN_TEMPLATES}

clean-local:
	rm -f os.cpp def.cpp admin_view.cpp
