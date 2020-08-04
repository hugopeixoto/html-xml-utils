:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo '<table border>
<thead>
<tr><th><th>head 1<th>head 2</tr>
<tbody>
<tr><th>head A<td>A1<td>A2</tr>
<tr><th>head B<td>B1<td>B2</tr>
<tr><th>head C<td>C1<td>C2</tr>
</table>' | ./hxtabletrans | ./hxtabletrans | ./hxtabletrans >$TMP1

echo '<table border>
<tr>
<th></th>
<th>head A</th>
<th>head B</th>
<th>head C</th>
</tr>
<tr>
<th>head 1</th>
<td>A1</td>
<td>B1</td>
<td>C1</td>
</tr>
<tr>
<th>head 2</th>
<td>A2</td>
<td>B2</td>
<td>C2</td>
</tr>
</table>' >$TMP2

cmp -s $TMP1 $TMP2


