:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' 'div + ul > li:nth-last-child(2n+1)' >$TMP1 <<-EOF
	<div><p>Not this.</p></div>
	<ul>
	<li>This</li>
	<li>Two</li>
	<li>This</li>
	<li>Four</li>
	<li>This</li>
	</ul>
	<address>But not this.</address>
	<div title="Not this either."/>
EOF
cat >$TMP2 <<-EOF
	<li>This</li>
	<li>This</li>
	<li>This</li>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
