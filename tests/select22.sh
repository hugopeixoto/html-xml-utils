:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/select18-XXXXXX` || exit 1
TMP2=`mktemp /tmp/select18-XXXXXX` || exit 1

./hxselect -s '\n' 'div:not(div div)' >$TMP1 <<-EOF
	<div id=d1>
	 <div id=d2 />
	 <div id=d3 />
	 <div id=d4 />
	</div>
EOF

cat >$TMP2 <<-EOF
	<div id="d1">
	 <div id="d2"></div>
	 <div id="d3"></div>
	 <div id="d4"></div>
	</div>
EOF

diff -u $TMP2 $TMP1
