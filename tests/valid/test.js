// tests/valid/test.js

function myTest() {
    let a = 1;
    if (a > 0) {
      a = a + (10 * 2); // a 现在是 21
    }

    // <-- 新添加的 while 循环 -->
    while (a > 18) {
        a = a - 1;
    }
    // <-- a 在这里应该是 18 -->

    return a;
}

myTest();