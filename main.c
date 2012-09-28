#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "comp.h"
#include "tmachine.h"
#include "lcalc.h"
#include "buf.h"
#include "comp_serialize.h"

static void
comp_test()
{
    {
        /*
         *  f(x, y) = x + y
         */

        struct node *f, **g;
        int y;

        g = node_array_new(2);
        g[0] = projection_node_new(0);
        g[1] = NULL;

        f = recursion_node_new(projection_node_new(0),
                               composition_node_new(successor_node_new(), g));
        //

        int x[2] = {5, 12};
        y = node_compute(f, x, 2);
        printf("y = %i\n", y);
        assert(y == 17);

        //

        node_destroy(f);
    }

    {
        /*
         *  f(x) = 2
         */

        struct node *f, **g, **h;
        int y;

        h = node_array_new(2);
        h[0] = zero_node_new();
        h[1] = NULL;

        g = node_array_new(2);
        g[0] = composition_node_new(successor_node_new(), h);
        g[1] = NULL;

        f = composition_node_new(successor_node_new(), g);

        int x[2] = {5, 12};
        y = node_compute(f, x, 2);
        printf("y = %i\n", y);
        assert(y == 2);

        node_destroy(f);
    }

    {
        /*
         *  f(x, y) = x * y
         */

        struct node *add, *mult, **g, **h;
        int y;

        g = node_array_new(2);
        g[0] = projection_node_new(0);
        g[1] = NULL;

        add = recursion_node_new(projection_node_new(0),
                                 composition_node_new(successor_node_new(), g));

        h = node_array_new(3);
        h[0] = projection_node_new(0);
        h[1] = projection_node_new(1);
        h[2] = NULL;

        mult = recursion_node_new(zero_node_new(), composition_node_new(add, h));
        int x[2] = {5, 6};
        y = node_compute(mult, x, 2);
        printf("y = %i\n", y);
        assert(y == 30);

        node_destroy(mult);
    }

    {
        /*
         *  f(x, y) = x ^ y
         */

        struct node *one, *add, *mult, *exp, **g, **h, **j, **k;
        int y;

        g = node_array_new(2);
        g[0] = projection_node_new(0);
        g[1] = NULL;

        add = recursion_node_new(projection_node_new(0),
                                 composition_node_new(successor_node_new(), g));

        h = node_array_new(3);
        h[0] = projection_node_new(0);
        h[1] = projection_node_new(1);
        h[2] = NULL;

        mult = recursion_node_new(zero_node_new(), composition_node_new(add, h));

        j = node_array_new(2);
        j[0] = zero_node_new();
        j[1] = NULL;

        one = composition_node_new(successor_node_new(), j);

        k = node_array_new(3);
        k[0] = projection_node_new(0);
        k[1] = projection_node_new(1);
        k[2] = NULL;

        exp = recursion_node_new(one, composition_node_new(mult, k));

        int x[2] = {4, 4};
        y = node_compute(exp, x, 2);
        printf("y = %i\n", y);
        assert(y == 256);

        node_destroy(exp);
    }

    {
        /*
         *  Factorial:
         *
         *  f(x) = x!
         */

        struct node *one, *add, *mult, *fact, **g, **h, **i, **j, **k;
        int y;

        g = node_array_new(2);
        g[0] = projection_node_new(0);
        g[1] = NULL;

        add = recursion_node_new(projection_node_new(0),
                                 composition_node_new(successor_node_new(), g));

        h = node_array_new(3);
        h[0] = projection_node_new(0);
        h[1] = projection_node_new(1);
        h[2] = NULL;

        mult = recursion_node_new(zero_node_new(), composition_node_new(add, h));

        k = node_array_new(2);
        k[0] = projection_node_new(1);
        k[1] = NULL;

        i = node_array_new(3);
        i[0] = projection_node_new(0);
        i[1] = composition_node_new(successor_node_new(), k);
        i[2] = NULL;

        j = node_array_new(2);
        j[0] = zero_node_new();
        j[1] = NULL;

        one = composition_node_new(successor_node_new(), j);

        fact = recursion_node_new(one, composition_node_new(mult, i));

        int x = 5;
        y = node_compute(fact, &x, 1);
        printf("y = %i\n", y);
        assert(y == 120);

        node_destroy(fact);
    }

    {
        /*
         *  Iszero:
         *
         *         | 1   if x = 0
         *  f(x) = | 0   if x > 0
         */

        struct node *iszero, *one, **j;
        int y;

        j = node_array_new(2);
        j[0] = zero_node_new();
        j[1] = NULL;

        one = composition_node_new(successor_node_new(), j);

        iszero = recursion_node_new(one, zero_node_new());

        int x = 0;
        y = node_compute(iszero, &x, 1);
        printf("y = %i\n", y);
        assert(y == 1);

        x = 5;
        y = node_compute(iszero, &x, 1);
        printf("y = %i\n", y);
        assert(y == 0);

        node_destroy(iszero);
    }

    {
        /*
         *  Predecessor:
         *
         *  f(x) = x - 1,      if x > 0
         *
         *  Monus:
         *
         *  f(x, y) = x - y,   if x > y
         */

        struct node **g, *monus, *pred;
        int y, x;

        pred = recursion_node_new(zero_node_new(), projection_node_new(1));

        x = 4;
        y = node_compute(pred, &x, 1);
        assert(3 == y);

        g = node_array_new(2);
        g[0] = projection_node_new(0);
        g[1] = NULL;

        monus = recursion_node_new(projection_node_new(0),
                                   composition_node_new(pred, g));

        int xs[2] = {17, 3};
        y = node_compute(monus, xs, 2);
        printf("y = %i\n", y);
        assert(14 == y);

        struct node *copy = node_clone(monus);
        node_destroy(copy);

        node_destroy(monus);
    }

    // divisibility, primes etc. !?

    printf("-------------------------\n"
           "All assertions passed ok.\n"
           "-------------------------\n");

    malloc_stats();
}

static void
tmachine_test()
{
    struct tm_machine *machine;
    struct tm_tape *tape;

    machine = t_machine_new();

    t_machine_insert_states(machine, 3);

    t_machine_add_instruction(machine, 0 /* q1 */, 0 /* q1 */, 1 /* a */, 1 /* a */, TM_RIGHT);
    t_machine_add_instruction(machine, 0 /* q1 */, 0 /* q1 */, 2 /* b */, 2 /* b */, TM_RIGHT);
    t_machine_add_instruction(machine, 0 /* q1 */, 1 /* q2 */, 0 /* - */, 1 /* a */, TM_LEFT);
    t_machine_add_instruction(machine, 1 /* q2 */, 1 /* q2 */, 1 /* a */, 1 /* a */, TM_LEFT);
    t_machine_add_instruction(machine, 1 /* q2 */, 1 /* q2 */, 2 /* b */, 2 /* b */, TM_LEFT);
    t_machine_add_instruction(machine, 1 /* q2 */, 2 /* q3 */, 0 /* - */, 0 /* - */, TM_RIGHT);


    tape = t_machine_tape_new();
    t_machine_tape_append_symbol(tape, 1);
    t_machine_tape_append_symbol(tape, 1);
    t_machine_tape_append_symbol(tape, 2);
    t_machine_tape_append_symbol(tape, 2);

    if (t_machine_run(machine, tape) < 0) {
        printf("Error!\n");
    }

    t_machine_tape_destroy(tape);
    t_machine_destroy(machine);

    malloc_stats();
}

static void
lcalc_test()
{
    /*
     *  λ0.λ1.1     (Church numeral zero)
     */
    struct lambda_term *zero = lambda_abstraction_new(0,
                                                      lambda_abstraction_new(1,
                                                                             lambda_variable_new(1)));

    /*
     *  λ2.λ3.λ4.3(234)     (Successor)
     */

    struct lambda_term *succ = lambda_abstraction_new(2,
                                                      lambda_abstraction_new(3,
                                                                             lambda_abstraction_new(4, lambda_application_new(lambda_variable_new(3),
                                                                                                                              lambda_application_new(lambda_application_new(lambda_variable_new(2),
                                                                                                                                                                            lambda_variable_new(3)),
                                                                                                                                                     lambda_variable_new(4))))));
    // ...

    struct lambda_term *a = lambda_application_new(succ, zero);

//    lambda_term_dump(a);

    lambda_term_normal_order_reduce_step(&a);
//    lambda_term_dump(a);

    lambda_term_normal_order_reduce_step(&a);
//    lambda_term_dump(a);

    lambda_term_normal_order_reduce_step(&a);
//    lambda_term_dump(a);


    // MPQ = (MP)Q

    // multiplication:
    //
    // λxyz.x(yz)           -> λ0.λ1.λ2.0(12)

    // c0 = λf.λx.x
    // c1 = λf.λx.fx
    // c2 = λf.λx.f(fx)     -> λ3.λ4.3(34)
    // c3 = λf.λx.f(f(fx))  -> λ5.λ6.5(5(56))

    // mult 2 3

    // (λ0.λ1.λ2.0(12))(λ3.λ4.3(34))(λ5.λ6.5(5(56)))

    // λ1.λ2.0(12)[0 := (λ3.λ4.3(34))(λ5.λ6.5(5(56)))]

    // λ1.λ2.(λ3.λ4.3(34))(λ5.λ6.5(5(56)))(12)



    /*
     *  λ0.λ1.λ2.0(12)
     */

    struct lambda_term *mult = lambda_abstraction_new(0,
                                                      lambda_abstraction_new(1,
                                                                             lambda_abstraction_new(2,
                                                                                                    lambda_application_new(lambda_variable_new(0),
                                                                                                                           lambda_application_new(lambda_variable_new(1),
                                                                                                                                                  lambda_variable_new(2))))));

    /*
     *  λ3.λ4.3(34)     (Church numeral 2)
     */

    struct lambda_term *church2 = lambda_abstraction_new(3,
                                                         lambda_abstraction_new(4,
                                                                                lambda_application_new(lambda_variable_new(3),
                                                                                                       lambda_application_new(lambda_variable_new(3),
                                                                                                                              lambda_variable_new(4)))));

    /*
     *  λ5.λ6.5(5(56))  (Church numeral 3)
     */

    struct lambda_term *church3 = lambda_abstraction_new(5,
                                                         lambda_abstraction_new(6,
                                                                                lambda_application_new(lambda_variable_new(5),
                                                                                                       lambda_application_new(lambda_variable_new(5),
                                                                                                                              lambda_application_new(lambda_variable_new(5),
                                                                                                                                                     lambda_variable_new(6))))));

    struct lambda_term *app = lambda_application_new(lambda_application_new(mult, church2),
                                                     church3);

    lambda_term_normal_order_reduce_step(&app);
//    lambda_term_dump(app);

    lambda_term_normal_order_reduce_step(&app);
//    lambda_term_dump(app);

    lambda_term_normal_order_reduce_step(&app);
//    lambda_term_dump(app);

    lambda_term_normal_order_reduce_step(&app);
//    lambda_term_dump(app);

    lambda_term_normal_order_reduce_step(&app);
    lambda_term_dump(app);

    //

//    struct buf *b = buf_new(64);
//    lambda_term_alpha_hash(app, b);
//    printf("%s\n", b->data);
//    buf_destroy(b);


    /*
     *  λ0.λ1.0 1     (Church numeral one)
     */

    struct lambda_term *cone = lambda_abstraction_new(0,
                                                      lambda_abstraction_new(1,
                                                                             lambda_application_new(lambda_variable_new(0),
                                                                                                    lambda_variable_new(1))));


    uint8_t x = lambda_term_alpha_compare(cone, a);

    printf("x = %u\n", x);

    //

    lambda_term_destroy(app);
    lambda_term_destroy(a);
    lambda_term_destroy(cone);

    // λ0.λ1.λ2.0 == λ4.λ5.λ6.4

    // λ0.λ1.λ2.0 == λ4.λ6.λ5.4

    //

    malloc_stats();
}

int
main(void)
{
    if (0 == 1)
        comp_test();        // tmp

//    if (0 == 1)
//        tmachine_test();    // tmp

//    if (0 == 1)
//        lcalc_test();

    struct node **g = node_array_new(2);
    g[0] = invalid_node_new();
    g[1] = NULL;

    struct node **h = node_array_new(2);
    h[0] = composition_node_new(projection_node_new(123), g);
    h[1] = NULL;

    //

//    struct node *n = composition_node_new(invalid_node_new(), h);
//    struct buf *b  = buf_new(64);

//    node_serialize(n, b);

//    int p = 0;
//    struct node *n1 = node_unserialize(b, &p);

//    struct buf *b2  = buf_new(64);
//    node_serialize(n1, b2);

//    printf("%s\n", b2->data);

    struct buf *b = buf_new(64);

//    buf_append_chars(b, "[0,0,0,0]");


    buf_append_chars(b, "[[<0,0>,0,{114}],0]");
//    buf_append_chars(b, "([0,0,<0,0>])");


    printf("valid = %i\n", node_serial_data_is_valid(b));

//    struct node *node = node_unserialize(b);

//    struct buf *buf2 = buf_new(64);
//    node_serialize(node, buf2);

//    printf("%s\n", buf2->data);



    struct node *n2 = recursion_node_new(zero_node_new(), zero_node_new());
    int x = node_compute(n2, NULL, 0);

    printf("w.. %d\n", x);


    return 0;
}
