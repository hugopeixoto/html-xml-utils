:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref -l >$TMP1 <<-EOF
	<p><dfn>@foo</dfn> and <dfn>foo</dfn>
	<span>@foo,</span> <em>foo.</em>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p><dfn id="atfoo">@foo</dfn> and <dfn id="foo">foo</dfn>
	<a href="#atfoo">@foo,</a> <a href="#foo"><em>foo.</em></a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
