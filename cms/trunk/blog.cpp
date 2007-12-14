void Blog::init()
{
	string root=global_config.sval("blog.script_path");

	fmt.media=global_config.sval("blog.media_path");

	url.add("^/?$",
		boost::bind(&Blog::main_page,this,"end"));
	fmt.main=root + "/";
	url.add("^/from/(\d+)$",
		boost::bind(&Blog::main_page,this,$1));
	fmt.main_from=root + "/from/%1%";
	url.add("^/post/(\d+)$",
		boost::bind(&Blog::post,this,$1));
	fmt.post=root + "/post/%1%";
	url.add("^/admin$",
		boost::bind(&Blog::admin,this));
	fmt.admin=root + "/admin"
	url.add("^/admin/new_post$",
		boost::bind(&Blog::new_post,this));
	fmt.new_post=root + "/admin/new_post";
	url.add("^/admin/edit_post/(\d+)$",
		boost::bind(&Blog::edit_post,this,$1));
	fmt.edit_post=root+"/adin/edit_post/%1%";
	url.add("^/admin/edit_comment/(\d+)$",
		boost::bind(&Blog::edit_comment,this,$1));
	fmt.edit_comment=root+"/adin/edit_comment/%1%";
	// All incoming information
	url.add("^/postback/comment$",
		boost::bind(&Blog::add_comment,this));
	fmt.add_comment=root+"/postback/comment";
	url.add("^/postback/post$",
		boost::bind(&Blog::add_post,this));
	fmt.add_post=root+"/postback/add_post";
	url.add("^/postback/approve$",
		boost::bind(&Blog::approve,this));
	fmt.approve=root+"/postback/approve";
}

void Boost::base_content(Content &c)
{
	option_t opt;

	options->get(BLOG_TITLE,opt);
	string title=opt.value;
	options->get(BLOG_DESCRIPTION,opt);
	string description=opt.value;

	c[TV_meida]=fmt.media;
	c[TV_title]=title;
	c[TV_description]=description;
}

void Blog::render_post(post_t const &p,bool disp_content,post_content_t &c)
{
	Text_Tool conv;
	conv.text2htmp(p.title,c.title);
	char const *ptr;
	
	if(p.abstract_id!=-1 && (ptr=texts->get(p.abstract_id))!=NULL) {
		conv.markdown2html(ptr,c.abstract);
	}
	if(disp_content && p.content_id!=-1 && (ptr=texts->get(p.content_id))!=NULL) {
		conv.markdown2html(ptr,c.content);
	}
	c.has_content=p.content_id!=-1;
	c.permlink=str(boost::format(fmt.post) % p.id);
	c.published=textdate(p.publish);
	
	user_t user;
	if(users->id.get(p.author_ud,user)) {
		c.author=user.username;
	}
	else {
		c.author="unknown author";
	}
}

void Blog::set_post_content(post_content_t &pc,Content &c,bool disp_content)
{
	if(disp_content) {
		c[TV_title]=pc.title.c_str();
	}
	else if(disp_content) {
		c[TV_more]=pc.has_content;
	}
	
	c[TV_author]=pc.author.c_str();
	if(has_content){
		c[TV_content]=pc.content.c_str();
	}
	else {
		c[TV_content].reset();
	}
	c[TV_published]=pc.published.c_str();
	c[TV_abstract]=pc.abstract.c_str();
	c[TV_permlink]=pc.permlink.c_str();
}

void Blog::post(string s_id)
{
	int id=atoi(s_id.c_str());
	Content c(T_VAR_NUM);
	base_content(c);
	post_t post;
	post_content_t pc;
	if(posts->id.get(id,post)) {
		c[TV_master_content]=TT_post;
		render_post(post,true,pc);
		set_post_content(pc,c);
	}
	else {
		c[TV_master_content]=TT_not_found;
	}
	
	Renderer view(templates,TT_master,c);
	while(view.redner(out)) {
		// Comments !!!
	}
}

void Blog::main_page(string from)
{
	Content c(T_VAR_NUM);
	base_content(c);
	c[TV_master_content]=TT_main_page;

	Posts::publish_c cur(*posts);

	Renderer view(templates,TT_master,c);

	int id;
	if(from=="end") {
		cur.end();
	}
	else {
		if(posts->get(atoi(from.c_str()),tmppost)) {
			// From the time of the last post
			cur.lte(tmppost.publish);
		}
		else {
			cur.end();
		}
	}

	int counter=0;
	post_content_t pc;
	while((id=view.render(out))!=0) {
		if(id==TV_get_post) {
			c[TV_next_post]=true;
			while(cur && !cur.is_open) {
				cur.next();
			}
			if(cur && counter<10) {
				post_t const &post=cur;
				render_post(post,false,pc);
				set_content(pc,c);
			}
			else if(cur && counter==10) {
				c[TV_next_post]=false;
				string link=str(format(fmt.main_from) % post.id);
				c[TV_next_page_link]=link;
			}
			else {
				c[TV_next_post]=false;
			}
			counter++;
			cur.next();
		}
	}

}