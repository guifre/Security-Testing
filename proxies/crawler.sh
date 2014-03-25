#!/bin/bash

browser='wget -qO- --user-agent="Mozilla/5.0 (Windows NT 5.2; rv:2.0.1) Gecko/20100101 Firefox/4.0.1"';
results="dirtyProxies.list";
debugFile="/tmp/crawlerdebug";
depth=3;
feedList="sources.list";

function crawl {
	res=$($browser $1);
	urls=`echo $res |  grep -o '<a href=['"'"'"][^"'"'"']*['"'"'"]' |   sed -e 's/^<a href=["'"'"']//' -e 's/["'"'"']$//'`;
	proxies=`echo $res | egrep -o '[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\:[0-9]{2,4}'`;
	echo "crawling $1 counting $2" >> $debugFile;
	for proxyItem in $proxies; do
		echo "$proxyItem" >> ${results};
		#echo "Found proxy[$proxyItem]";
		printf ".";
	done
	for target in $urls; do
		if [ $2 -lt ${depth} ]; then
			counter=$[$2+1];
			case "$target" in
				http* ) newTarget=$target ;;
				 .\/* ) newTarget=$3"/"$target ;;
				 www* ) newTarget="http://$target" ;;
				  \/* ) newTarget="$3$target" ;;
				    * ) newTarget="$3/$target" ;;
			esac
			#echo "Target [$target] host [$3] => New target [$newTarget]";
			crawl $newTarget $counter $3 &
			sleep 10;
		fi
	done
}

while read line; do
	host=`echo $3 | awk -F/ '{print $3}'`;
	crawl $line"/" 1 $line $line &
	sleep 1;
done < "${feedList}"
