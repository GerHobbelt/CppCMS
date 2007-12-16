//
// C++ Implementation: views
//
// Description: 
//
//
// Author: artik <artik@art-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//


void View_Comment::init(comment_t &c)
{
	Text_Tool tt;
	user_t user;
	if(c.author_id!=-1 && users->get(id,user)) {
		tt.text2html(user.username,author);
	}
	else if(!(c.author=="")) {
		
	}
}