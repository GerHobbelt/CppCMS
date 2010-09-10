#!/usr/bin/perl
use Digest::MD5 qw(md5 md5_hex md5_base64);

$count = 0;
$toc = '';
$text = '';
$toc_level = 0;
while(<>) {
	if(/^(.*)<h([1-6])>([^<>]+)<\/h[1-6]>(.*)$/) {
		$start = $1;
		$end = $4;
		$count ++;
		$level = int($2);
		$title = $3;
		$id = md5_hex($title+$count);
		$text = $text . "$start<h$level id=\"$id\">$title</h$level>$end\n";
		while($toc_level < $level) {
			$toc = $toc . "<ul>\n";
			$toc_level++;
		}
		while($level < $toc_level) {
			$toc = $toc . "</ul>\n";
			$toc_level--;
		}
		$toc = $toc . "<li><a href=\"#$id\">$title</a></li>\n";
		
	}
	else {
		$text = $text . $_;
	}
}

while($toc_level > 0) {
	$toc = $toc . "</ul>\n";
	$toc_level--;
}

print $toc;
print $text;

