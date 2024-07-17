# NBLang: The Vietnamese Programming Language

This is a fork of the [NIMBLE](https://github.com/namberino/nimble) interpreter. This fork aims to implement the NIMBLE programming language but with Vietnamese syntax

```nblang
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
        nếu (đây.trái == không)
        {
            trả đây.vật;
        }

        trả đây.vật + đây.trái.check() - đây.phải.check();
    }
}
```
