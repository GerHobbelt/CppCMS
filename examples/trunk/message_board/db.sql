CREATE TABLE messages(
   id integer  primary key autoincrement not null,
   topic_id integer,
   title varchar(256),
   content varchar(256)
);
CREATE TABLE topics (
  id  integer  primary key autoincrement not null,
  title varchar(256)
);
CREATE INDEX topic_ind on messages(topic_id,id);
