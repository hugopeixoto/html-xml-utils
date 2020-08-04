:
trap 'rm -r $DIR' 0
DIR=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1

p=$PWD
cd $DIR
echo '<p><dfn>foo1</dfn> and <dfn>foo2</dfn><p><em>bar1</em> and <em>bar2</em>' >in1
echo '<p><dfn>bar1</dfn> and <dfn>bar2</dfn><p><em>foo1</em> and <em>foo2</em>' >in2

$p/hxref -i index -b out1 in1 >/dev/null
$p/hxref -i index -b out2 in2 >out2
$p/hxref -i index -b out1 in1 >out1

# Add a newline
echo '' >>out1
echo '' >>out2

echo '<html><body><p><dfn id="foo1">foo1</dfn> and <dfn id="foo2">foo2</dfn></p><p><a href="out2#bar1"><em>bar1</em></a> and <a href="out2#bar2"><em>bar2</em></a>
</p></body></html>' >ref1
echo '<html><body><p><dfn id="bar1">bar1</dfn> and <dfn id="bar2">bar2</dfn></p><p><a href="out1#foo1"><em>foo1</em></a> and <a href="out1#foo2"><em>foo2</em></a>
</p></body></html>' >ref2

cmp -s out1 ref1 && cmp -s out2 ref2
