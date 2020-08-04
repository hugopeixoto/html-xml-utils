:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxselect -s '\n' div.this p >$TMP1 <<-EOF
	<div><p>Not this.</p></div>
	<div class="this">
	<p>Select this.</p>
	<address>But not this.</address>
	</div>
	<div title="Not this either."/>
EOF
cat >$TMP2 <<-EOF
	<p>Select this.</p>
EOF

diff -u $TMP1 $TMP2
# cmp -s $TMP1 $TMP2
