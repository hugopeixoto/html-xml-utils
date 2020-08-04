:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo '<table border>
<thead>
<tr><th><!--preserve this--><th>head 1<th>head 2</tr>
<tbody>
<tr><th>head A<td>→<td>↓</tr>
<tr><th>head B<td>↑<td>←</tr>
<tr><th>head C<td> ⋯ <td> ⋯ </tr>
</table>' | ./hxtabletrans -c >$TMP1

echo '<table border>
<tr>
<th><!--preserve this--></th>
<th>head A</th>
<th>head B</th>
<th>head C</th>
</tr>
<tr>
<th>head 1</th>
<td>↓</td>
<td>←</td>
<td> ⋮ </td>
</tr>
<tr>
<th>head 2</th>
<td>→</td>
<td>↑</td>
<td> ⋮ </td>
</tr>
</table>' >$TMP2

cmp -s $TMP1 $TMP2
