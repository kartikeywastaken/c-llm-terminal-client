#include <stdio.h>

typedef struct {
    int age;
} Person;

// This function receives a COPY of the person.
void haveBirthday_WRONG(Person p) {
    p.age = p.age + 1; // This only changes the copy.
    printf("Inside the 'wrong' function, age is now %d.\n", p.age);
}

// This function receives the ADDRESS of the original person.
void haveBirthday_CORRECT(Person *p_ptr) {
    // Use the -> operator to access members via a pointer.
    p_ptr->age = p_ptr->age + 1; // This changes the original data.
    printf("Inside the 'correct' function, age is now %d.\n", p_ptr->age);
}

int main() {
    Person kartikey = { 25 };
    printf("Original age is %d.\n", kartikey.age);

    // --- The WRONG way ---
    haveBirthday_WRONG(kartikey);
    printf("After the 'wrong' function, age is still %d.\n\n", kartikey.age);

    // --- The CORRECT way ---
    // Pass the memory address of 'kartikey' using the '&' operator.
    haveBirthday_CORRECT(&kartikey);
    printf("After the 'correct' function, age is now %d.\n", kartikey.age);

    return 0;
}