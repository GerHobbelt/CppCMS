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
	fmt.admin=root + "/admin/new_post";
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
	Options::cursor_t cur(*options);
	
	cur==option_t::BLOG_TITLE;
	option_t opt=cur;
	title=opt.value;
	
	cur==option_t::BLOG_DESCRIPTION;
	opt=cur;
	description=opt.value;
	
	c[V_meida]=fmt.media;
	c[V_title]=title;
	c[V_description]=description;
}

void Boost::main_page()
{
	Content c;
	base_content(c);
	Posts::publish_c cur(*posts);
	
	
}