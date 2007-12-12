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
	title=opt.value;
	options->get(BLOG_DESCRIPTION,opt);
	description=opt.value;

	c[TV_meida]=fmt.media;
	c[TV_title]=title;
	c[TV_description]=description;
}

void Blog::render_post(post_t const &p,bool disp_content,Content &c)
{
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
		cur.lte(atoi(from.c_str()));
	}

	int counter=0;
	while((id=view.render(out))!=0) {
		if(id==TV_get_post) {
			c[TV_next_post]=true;
			if(cur && counter<10) {
				post_t const &post=cur;
				render_post(post,false,c);
			}
			else if(cur && counter==10) {
				c[TV_next_post]=false;
				string link=str(format(fmt.main_from) % post.publish);
				c[TV_next_page_link]=link;
			}
			else {
				c[TV_next_post]=false;
			}
			counter++;
		}
	}

}