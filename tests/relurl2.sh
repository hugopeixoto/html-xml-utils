:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxwls -l -b "http://example.org/remove/this" >$TMP1 <<-EOF
	<link rel=rel href="/link0">
	<base href="http://user:pass@[1234:abcd]:90/base/segm?query1">
	<a href="link1">link1</a>
	<a href="/link2">link2</a>
	<a href="http://other.com/base3/link3">link3</a>
	<a href="query?query2">query</a>
	<a href="?query3">query</a>
	<a href="">self</a>
	<a href="#fragment">self</a>
	<a href="../link4">link4</a>
	<a href="../../link5">link5</a>
	<img src="img.png" alt="img">
	<a href="./">link6</a>
	<a href="./link7">link7</a>
	<a href="../.././link7">link8</a>
EOF
cat >$TMP2 <<-EOF
	link	rel	http://example.org/link0
	base		http://user:pass@[1234:abcd]:90/base/segm?query1
	a		http://user:pass@[1234:abcd]:90/base/link1
	a		http://user:pass@[1234:abcd]:90/link2
	a		http://other.com/base3/link3
	a		http://user:pass@[1234:abcd]:90/base/query?query2
	a		http://user:pass@[1234:abcd]:90/base/segm?query3
	a		http://user:pass@[1234:abcd]:90/base/segm?query1
	a		http://user:pass@[1234:abcd]:90/base/segm?query1#fragment
	a		http://user:pass@[1234:abcd]:90/link4
	a		http://user:pass@[1234:abcd]:90/link5
	img		http://user:pass@[1234:abcd]:90/base/img.png
	a		http://user:pass@[1234:abcd]:90/base/
	a		http://user:pass@[1234:abcd]:90/base/link7
	a		http://user:pass@[1234:abcd]:90/link7
EOF

cmp -s $TMP1 $TMP2
