///
/// Author: Artyom Beilis
///
///
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <Magick++.h>
#include <list>
#include <ctime>
#include <booster/posix_time.h>
#include <cppcms/urandom.h>

#include <cppcms/application.h>
#include <cppcms/http_response.h>
#include <cppcms/session_interface.h>
#include <cppcms/url_mapper.h>

#include <apps/captcha.h>

namespace { // anonymous

char const choise[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

class captcha_maker {
	captcha_maker(captcha_maker const &);
	void operator=(captcha_maker const &);
public:
	captcha_maker(int width_,int height_) :
		width(width_),
		height(height_),
		lw(height_/2),
		lh(height_/2)
	{
		using namespace Magick;
		InitializeMagick("");
		letters.resize(sizeof(choise)-1,Magick::Image(Magick::Geometry(lw,lh),Magick::Color("white")));
		for(int i=0;i<int(sizeof(choise))-1;i++) {
			Image &image=letters[i];
			image.type( GrayscaleType );
			image.strokeColor("black"); // Outline color
			image.strokeWidth(1);
			image.fillColor("black");
			image.font("helvetica");
			int dx=width/7;
			image.fontPointsize(dx*3/2);
			std::string l;
			l+=choise[i];
			
			std::list<Magick::Drawable> lst;
			lst.push_back(DrawableGravity(CenterGravity));
			lst.push_back(DrawableText(0,0,l));
			
			image.draw(lst);
		}

	}
	std::string create_captcha_line() const
	{
		cppcms::urandom_device r;
		unsigned seed;
		r.generate(&seed,sizeof(seed));
		return create_captcha_line(&seed);
	}

	void create_captcha(std::string str,std::ostream &file) const
	{
		cppcms::urandom_device r;
		unsigned seed;
		r.generate(&seed,sizeof(seed));
		create_captcha(str,file,&seed);
	}
private:
	int randlim(unsigned *seed,unsigned limit) const
	{
		return int(double(rand_r(seed))/(RAND_MAX+1.0)*limit);
	}

	std::string create_captcha_line(unsigned *seed) const
	{
		std::string letters;
		for(unsigned i=0;i<6;i++) {
			char l=choise[randlim(seed,sizeof(choise)-1)];
			letters+=l;
		}
		return letters;
	}

	void create_captcha(std::string str,std::ostream &file,unsigned *seed) const
	{
		using namespace Magick;
		Image image( Geometry(width,height) ,Color("white"));

		// Set draw options
		image.type( GrayscaleType );
		image.strokeColor(Color(0,0,0)); // Outline color
		image.strokeWidth(1);
		image.fillColor("none");
		image.matte(false);


		int dx=width/(str.size()+1);
		image.negate();

		for(unsigned i=0;i<str.size();i++) {
			int x=int(i)*dx + dx/2 - lw/2 + randlim(seed,lw/4)-lw/8;
			int y=height/2-lh/2 +randlim(seed,lh/2)-lh/4;

			char l=str.at(i);
			int pos=0;

			if('A'<=l && l<='Z') 
				pos=l-'A';
			else if('a'<=l && l<='z')
				pos=l-'a'+('z'-'a'+1);
			else 
				pos=l-'0'+('z'-'a'+1)*2;

			Image tmp=letters.at(pos);
			tmp.rotate(randlim(seed,50)-25);
			tmp.negate();
			image.composite(tmp,x,y,PlusCompositeOp);
		}
		image.negate();
		image.strokeWidth(height/50.0);
		/*for(int times=0;times<2;times++) {
			std::list<Magick::Coordinate> coord;
			for(unsigned i=0;i<str.size();i++) {
				int x=int(i)*dx + dx/2  + randlim(seed,lw/2)-lw/4;
				int y=height/2 +randlim(seed,lh)-lh/2;
				coord.push_back(Coordinate(x,y));
			}
			image.draw(DrawablePolyline(coord));
		}*/
		/*static const double kern[]=
			{ 0.1, 0,   0.1,
			  0,   0.6, 0,
			  0.1, 0,   0.1 };
		image.convolve(3,kern);*/

		Blob b;
		image.quality(25);
		image.write(&b,"png");

		file.write((char*)b.data(),b.length());

	}
private:
	int width;
	int height;
	int lw,lh;
	std::vector<Magick::Image> letters;
};

captcha_maker maker_instance(120,60);

} // anon

namespace apps {

	captcha::captcha(cppcms::service &srv) : cppcms::application(srv)
	{
		mapper().assign(""); // default empty url
	}

	void captcha::main(std::string /*url*/)
	{
		std::string code = maker_instance.create_captcha_line();
		session().set("captcha",code);
		session().on_server(true);
		response().content_type("image/png");
		maker_instance.create_captcha(code,response().out());
	}

}

