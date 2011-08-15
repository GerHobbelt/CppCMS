begin;
update post2cat set is_open=(select posts.is_open from posts where posts.id=post2cat.post_id);
update text_options set value='3' where id='dbversion';
commit;
