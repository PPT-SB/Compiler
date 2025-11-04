// tests/valid/test.js

function myTest() {
    let a = 1;
    if (a > 0) {
      a = a + (10 * 2); // a 现在是 21
    }
 
 // a 在这里是 21
 for (let i = 0; i < 3; i = i + 1) {
     a = a - 1;
    }
 // a 现在应该是 18
    return a;
}

myTest();