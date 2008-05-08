pkgdata_DATA = look.opcode style.css adminstyle.css

TEMPLATES= lnglst.tmpl admin_editpost.tmpl admin_main.tmpl error.tmpl main_page.tmpl post.tmpl\
	admin_login.tmpl admin.tmpl feed_posts.tmpl master.tmpl sidebar.tmpl\
	admin_editcomment.tmpl 

look.opcode: ${TEMPLATES}
	tmpl_bld -o look.opcode -p messages.pot ${TEMPLATES}

clean-local:
	rm -f look.opcode look.s messages.pot
