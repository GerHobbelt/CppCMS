#ifndef BLOG_ALL_H
#define BLOG_ALL_H

#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <iostream>
#include <memory>
#include <list>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <cgicc/HTTPRedirectHeader.h>

#include <dbi/dbixx.h>

#include <tmpl/renderer.h>
#include <tmpl/content.h>

#include <cppcms/text_tool.h>
#include <cppcms/worker_thread.h>
#include <cppcms/thread_pool.h>
#include <cppcms/global_config.h>
#include <cppcms/url.h>
#include <cppcms/templates.h>

#endif
