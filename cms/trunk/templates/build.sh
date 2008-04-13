#!/bin/sh
if tmpl_bld look.opcode admin_editpost.tmpl admin_login.tmpl admin_main.tmpl error.tmpl feed_posts.tmpl main_page.tmpl post.tmpl sidebar.tmpl master.tmpl admin.tmpl ; then
	echo Complete
	exit 0
else
	echo Failed
	exit 0
fi
