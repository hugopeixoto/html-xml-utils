:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' 'address:only-of-type' >$TMP1 <<-EOF
	<div>
	<p>Not this.</p>
	<ul>
	<li>One</li>
	<li>Two</li>
	<li>Three</li>
	<li>Four</li>
	<li>Five</li>
	</ul>
	<address>This.</address>
	<div title="Not this one."/>
	<div title="Nor this one."></div>
	</div>
EOF
cat >$TMP2 <<-EOF
	<address>This.</address>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
