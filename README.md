# Namlang: The Vietnamese Programming Language

This is a fork of the [NIMBLE](https://github.com/namberino/nimble) interpreter. This fork aims to implement the NIMBLE programming language but with Vietnamese syntax

```nimble
lớp Tree
{
    init(vật, độ_sâu)
    {
        this.vật = vật;
        this.độ_sâu = độ_sâu;

        nếu (độ_sâu > 0)
        {
            biến vật2 = vật + vật;
            độ_sâu = độ_sâu - 1;
            this.trái = Tree(vật2 - 1, độ_sâu);
            this.phải = Tree(vật2, độ_sâu);
        }
        thì
        {
            this.trái = không;
            this.phải = không;
        }
    }

    check()
    {
        nếu (this.trái == không)
        {
            trả this.vật;
        }

        trả this.vật + this.trái.check() - this.phải.check();
    }
}
```
