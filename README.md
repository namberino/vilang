# Vilang: The Vietnamese Programming Language

This is a programming language with support for Vietnamese syntaxes.

## Syntax

```vilang
lớp Tree
{
    tạo(vật, độ_sâu)
    {
        đây.vật = vật;
        đây.độ_sâu = độ_sâu;

        nếu (độ_sâu > 0)
        {
            biến vật2 = vật + vật;
            độ_sâu = độ_sâu - 1;
            đây.trái = Tree(vật2 - 1, độ_sâu);
            đây.phải = Tree(vật2, độ_sâu);
        }
        thì
        {
            đây.trái = không;
            đây.phải = không;
        }
    }

    check()
    {
        nếu (đây.trái == trống)
        {
            trả đây.vật;
        }

        trả đây.vật + đây.trái.check() - đây.phải.check();
    }
}
```

## Building and running

You can build this by running `make compile` and run this by running `./bin/vilang <filename>.vila`
