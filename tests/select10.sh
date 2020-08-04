:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' 'li:nth-child(3)' >$TMP1 <<-EOF
	<div><p>Not this.</p></div>
	<ul>
	<li>One</li>
	<li>Two</li>
	<li>Select this</li>
	<li>Four</li>
	</ul>
	<address>But not this.</address>
	<div title="Not this either."/>
EOF
cat >$TMP2 <<-EOF
	<li>Select this</li>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
