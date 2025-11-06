// tests/valid/test.js

function myTest() {
    let a = 1;
    if (a > 0) {
      a = a + (10 * 2); // a 现在是 21
    }
 
 // a 在这里是 21
 for (let i = 0; i < 3; i++) {
    if (i == 1) {
        break; // 跳过 i=1 的情况
    }
    a = a - 1;
}
    ++a;
 // a 现在应该是 20
    return a;
}

myTest();