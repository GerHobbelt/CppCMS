pkgdata_DATA = look.opcode style.css adminstyle.css

TEMPLATES=admin_editpost.tmpl admin_main.tmpl error.tmpl main_page.tmpl post.tmpl\
	admin_login.tmpl admin.tmpl feed_posts.tmpl master.tmpl sidebar.tmpl

look.opcode: ${TEMPLATES}
	tmpl_bld look.opcode ${TEMPLATES}

clean-local:
	rm -f look.opcode look.s
