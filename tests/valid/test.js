
function myTest() {
    let a = 1;
    if (a > 0) {
      a = a + (10 * 2); // a 现在是 21
    }

    for (let i = 0; i < 3; i++) {
        if (i == 1) {
            break; // 只有 i=0 会执行 (a 变为 20)
        }
        a = a - 1;
    }
    // a 在这里应该是 20

    ++a; // a 变为 21

    // <-- 新添加的 switch 语句 -->
    switch (a) {
        case 10:
            a = 100;
            break;
        case 21:
            a = 50;
            break;
        default:
            a = 0;
    }
    // <-- a 在这里应该是 50 -->

    return a;
}

myTest();