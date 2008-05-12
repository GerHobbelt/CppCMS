pkgdata_DATA = look.opcode style.css adminstyle.css os.opcode

TEMPLATES= lnglst.tmpl admin_editpost.tmpl admin_main.tmpl error.tmpl main_page.tmpl page.tmpl post.tmpl\
	admin_login.tmpl admin.tmpl feed_comments.tmpl feed_posts.tmpl  sidebar.tmpl\
	admin_editcomment.tmpl setup_blog.tmpl

look.opcode: ${TEMPLATES} master-def.tmpl
	tmpl_bld -o look.opcode -p messages.pot ${TEMPLATES} master-def.tmpl

os.opcode: ${TEMPLATES} master-os.tmpl
	tmpl_bld -o os.opcode -p messages-os.pot ${TEMPLATES} master-os.tmpl

clean-local:
	rm -f look.opcode look.s messages.pot os.opcode os.s messages-os.pot
