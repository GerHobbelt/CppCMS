begin;
insert into text_options values('blog_title',(select value from options where id=0)); 
insert into text_options values('blog_description',(select value from options where id=1)); 
insert into text_options values('contact',(select value from options where id=2)); 
update text_options set value='2' where id='dbversion';
drop table options;
commit;
