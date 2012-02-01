begin;
select setval('users_id_seq',(select max(id) from users));
select setval('posts_id_seq',(select max(id) from posts));
select setval('comments_id_seq',(select max(id) from comments));
select setval('pages_id_seq',(select max(id) from pages));
select setval('links_id_seq',(select max(id) from links));
select setval('cats_id_seq',(select max(id) from cats));
commit;
