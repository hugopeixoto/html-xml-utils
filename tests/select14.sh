:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' ':empty' >$TMP1 <<-EOF
	<div><p>Not this.</p></div>
	<ul>
	<li>One</li>
	<li>Two</li>
	<li>Three</li>
	<li>Four</li>
	<li>Five</li>
	</ul>
	<address>But not this.</address>
	<div title="This one."/>
	<div title="And this one."></div>
EOF
cat >$TMP2 <<-EOF
	<div title="This one."></div>
	<div title="And this one."></div>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
