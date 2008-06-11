select setval('users_id_seq',(select max(id)+1 from users));
select setval('posts_id_seq',(select max(id)+1 from posts));
select setval('comments_id_seq',(select max(id)+1 from comments));
select setval('pages_id_seq',(select max(id)+1 from pages));
select setval('links_id_seq',(select max(id)+1 from links));
select setval('cats_id_seq',(select max(id)+1 from cats));
