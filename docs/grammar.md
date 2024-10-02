# Luật cú pháp

Luật cú pháp của Vilang sẽ sử dụng quy tắc ngôn ngữ hình thức (*formal grammar*).  Quy tắc ngôn ngữ hình thức là một tập các chuỗi (*string*) được xây dựng dựa trên một bảng chữ cái (*alphabet*, có thể là tập các ký tự), và chúng được ràng buộc bởi các luật (*rule*) hoặc ngữ pháp (*grammar*) đã được định nghĩa trước. Luật cú pháp này sẽ giúp tìm ra các chuối đó trong mã nguồn.  

2 loại:
- Terminal: Một kí tự từ bảng chứ cái của luật cú pháp (VD: `nếu`, `"hello"`,...).
- Nonterminal: Một giá trị tham chiếu tới một quy luật khác trong luật cú pháp.

Các luật cú pháp là nền tảng cho cú pháp và AST của ngôn ngữ lập trình 

Để có thể tránh sự mơ hồ trong thứ tự thực hiện phép tính, parser sẽ cần xác định được thứ tự ưu tiên (*precedence*) và tính kết hợp (*associativity*). Thứ tự ưu tiên sẽ giúp tìm ra phần tử nào được đánh giá đầu tiên trong một biểu thức. Tính kết hợp có nghĩa là phần tử nào được đánh giá đầu tiên trong một chuỗi các phần tử giống nhau.

Ví dụ:
- Kết nối bên trái: `4 - 2 - 1` = `(4 - 2) - 1`.
- Kết nối bên phải: `a = b = c` = `a = (b = c)`.

## Luật cú pháp của Vilang

- Literals: *số*, *chuỗi*, *boolean*, *trống*.
- Unary: `'!'` ở trước để thực hiện `not`, `-` ở trước để tạo số âm.
- Binary: Phép tính `+`, `-`, `*`, `/` và phép so sánh `==`, `!=`, `<`, `<=`, `>`, `>=`.
- Parentheses: `'('` và `')'` ở quanh 1 biểu thức.

Ví dụ:
```
1 - (2 * 3) < 4 == false
```

Tính kết hợp:

| Tên | Phép tính | Kết hợp với |
| --- | --- | --- |
| Equality | ==, != | Trái |
| Comparison | >, >=, <, <= | Trái |
| Term | -, + | Trái |
| Factor | /, * | Trái |
| Unary | !, - | Phải |

Note: Luật ngữ pháp được viết theo dạng [Backus–Naur](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form)

Luật cú pháp biểu thức (expression):
```
expression := assignment
assignment := ( call "." )? IDENTIFIER ( "=" | "+=" | "-=" | "*=" | "/=" ) assignment | logic_or
logic_or := logic_and ( "or" logic_and )*
logic_and := equality ( "and" equality )*
equality := comparison ( ( "!=" | "==" ) comparison )*
comparison := exponent ( ( ">" | ">=" | "<" | "<=" ) exponent )*
exponent := term ( "**" term )*
term := factor ( ( "-" | "+" ) factor )*
factor := unary ( ( "/" | "*" | "%" ) unary )*
unary := ( "!" | "-" ) unary | call
call := subscript ( "(" arguments? ")" | "." IDENTIFIER )*
subscript := primary ( "[" arguments? "]" )*
arguments := expression ( "," expression )*
primary := NUMBER | STRING | IDENTIFIER | "true" | "false" | "nil" | "this" | "(" expression ")" | "super" "." IDENTIFIER | "[" (expression)* "]"
```

Luật cú pháp lệnh (statement):
```
program := declaration* EOF
declaration := mut_declaration | function_declaration | class_declaration | statement
statement := expression_statement | print_statement | if_statement | for_statement | while_statement | break_statement | return_statement | import_statement | block
expr_statement := expression ";"
print_statement := "in" "(" expression ")" ";"
mut_declaration := "biến" IDENTIFIER ( "=" expression )? ";"
function_declaration := "hàm" function
class_declaration := "lớp" IDENTIFIER ( ":" IDENTIFIER )? "{" function* "}"
function := IDENTIFIER "(" parameters? ")" block
parameters := IDENTIFIER ( "," IDENTIFIER )*
block := "{" declaration* "}"
if_statement := "nếu" "(" expression ")" statement ( "thì" statement )?
for_statement := "với" "(" ( mut_declaration | expression_statement | ";" ) expression? ";" expression? ")" statement
while_statement := "khi" "(" expression ")" statement
break_statement := "dừng" ";"
return_statement := "trả" expression? ";"
import_statement := "nhập" STRING ";"
```
