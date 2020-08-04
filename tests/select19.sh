:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/select18-XXXXXX` || exit 1
TMP2=`mktemp /tmp/select18-XXXXXX` || exit 1

./hxselect -s '\n' 'a:not(div a)' >$TMP1 <<-EOF
	<div>
	<a>First</a>
	</div>
	<p>
	<a>Second</a>
	<a>Third</a>
	<a>Fourth</a>
	</p>
EOF

cat >$TMP2 <<-EOF
	<a>Second</a>
	<a>Third</a>
	<a>Fourth</a>
EOF

diff -u $TMP2 $TMP1
