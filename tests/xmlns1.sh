:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxxmlns >$TMP1 <<-EOF
	<outer>
	 <a xmlns="x:y" f='f'>
	  <b:b xmlns:b="p:q" g='g'>
	   <a h="h"   />
	   <b:b/>
	   <xml:c/>
	   <!--comment-->
	  </b:b>
	 </a>
	</outer>
EOF
cat >$TMP2 <<-EOF
	<{}outer>
	 <{x:y}a f="f">
	  <{p:q}b g="g">
	   <{x:y}a h="h"/>
	   <{p:q}b/>
	   <{http://www.w3.org/XML/1998/namespace}c/>
	   <!--comment-->
	  </{p:q}b>
	 </{x:y}a>
	</{}outer>
EOF

cmp -s $TMP1 $TMP2
