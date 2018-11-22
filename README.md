# MoveEllisionBenchmark
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## What is this?

This project has been made to reveal an inconsistency in the MSVC x86 compiler. It uses [picobench](https://github.com/iboB/picobench) and cmake to provide an analysis against MSVC and Clang (x86 and amd64).

## Description
Recently, I have been benchmarking the effect of using ```operator-``` (and other similar operator overloads) with a class.

Given a class like the following:

```c++
class Obj
{
    double x;
    double y;
    double z;
    double w;

    Obj operator- (const Obj& RHS)
    {
        return {x - RHS.x, y - RHS.y, z - RHS.z, w - RHS.w};
    }
};
```
Should I prefer to use the objects members directly? Is the compiler going to be able to optimize the returned object from ```operator-``` using Return Value Optimization (RVO)? This makes me wonder about the following three examples: 

Example 1:
```c++
Obj A, B, C; //initalized with data at some point

C.x = A.x - B.x;
C.y = A.y - B.y;
C.z = A.z - B.z;
C.w = A.w - B.w;
```
Certainly this looks fast, there are no "extra" variables anywhere, we explicitly state that the operation of A - B is happening in C. 

Example 2:
```c++
Obj A, B; //initialized with data at some point

Obj C = A - B;
```
This also looks good, clearly A - B return a temporary object, but certainly compilers can see that it is just going to describe C at the end of the day so it just elides the temporary.

Example 3:
```c++
Obj A, B, C; //A and B initialized with data at some point.

C = A - B;
```
Very similar to  Example 2, but C has already been made, it has data now, therefore a logical conclusion is we must copy each value of the temp into C. Which should be in this case ```4 * sizeof(double)``` bytes of data.

To me, this seems reasonable; however, the conclusion does not. If Example 3 is truly slower than Example 1. It almost naturally suggests no-one should be using ```operator-``` in these trivial cases because we can easily "beat" the compiler.

## Results

This repository hosts my experiment. I test floats, vs doubles, vs ints, vs longs. I also test MSVC x86, vs MSVC x64, vs Clang 7.0.0 x86, vs Clang 7.0.0 x64. I compile using VS Code with the cmake extension provided by [@vector-of-bool](https://github.com/vector-of-bool).

As mentioned, I am using [picobench](https://github.com/iboB/picobench) to measure. Shout out to [Borislav Stanimirov (@idoB)](https://github.com/iboB) for such a useful header only library!

The following results show 4 benchmarks, the Orig is my Example 1, the Normal is my Example 2, the Slow is my Example 3, Moved is just like Example 2 but performs ```Obj C = std::move(A - B)```

The following results were taken by running in the default release build on Windows 10. My CPU is an i7-4790 @ 3.60 GHz.

### MSVC x86:
Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
DoubleBenchmark::Orig * |       5 |        - | 182052188.7
DoubleBenchmark::Normal |       7 |    1.400 | 138494975.4
DoubleBenchmark::Moved |       7 |    1.400 | 136402170.2
DoubleBenchmark::Slow |       7 |    1.400 | 136815365.2

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
FloatBenchmark::Orig * |       5 |        - | 181321312.9
FloatBenchmark::Normal |       7 |    1.400 | 137650782.8
FloatBenchmark::Moved |       7 |    1.400 | 141532980.7
FloatBenchmark::Slow |       7 |    1.400 | 141532980.7

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
IntBenchmark::Orig * |       3 |        - | 265588247.4
IntBenchmark::Normal |       3 |    1.000 | 280435729.8
IntBenchmark::Moved |       4 |    1.333 | 241442049.8
IntBenchmark::Slow |       4 |    1.333 | 242739684.7

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
LongBenchmark::Orig * |       3 |        - | 265588247.4
LongBenchmark::Normal |       3 |    1.000 | 293178453.5
LongBenchmark::Moved |       3 |    1.000 | 250837945.3
LongBenchmark::Slow |       3 |    1.000 | 250837945.3

Woah, looks like I was *mostly* right, the Slow case shows it is slower frequently (but not always?).

**Note**: the Moved and Slow benchamraks compile to be the same function.
### MSVC amd64:
Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
DoubleBenchmark::Orig * |       5 |        - | 182052188.7
DoubleBenchmark::Normal |       5 |    1.000 | 179160983.2
DoubleBenchmark::Moved |       5 |    1.000 | 182052188.7
DoubleBenchmark::Slow |       5 |    1.000 | 182052188.7


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
FloatBenchmark::Orig * |       5 |        - | 182052188.7
FloatBenchmark::Normal |       5 |    1.000 | 182788980.4
FloatBenchmark::Moved |       5 |    1.000 | 181321312.9
FloatBenchmark::Slow |       5 |    1.000 | 181321312.9


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
IntBenchmark::Orig * |       3 |        - | 264035609.5
IntBenchmark::Normal |       2 |    0.667 | 355511365.2
IntBenchmark::Moved |       2 |    0.667 | 334450593.7
IntBenchmark::Slow |       2 |    0.667 | 334450593.7


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
LongBenchmark::Orig * |       3 |        - | 264035609.5
LongBenchmark::Normal |       2 |    0.667 | 355511365.2
LongBenchmark::Moved |       2 |    0.667 | 339478334.3
LongBenchmark::Slow |       2 |    0.667 | 339469381.3

Wait...somehow in MSVC amd64 there is **no** slowdown. In fact we see speed ups?!

**Note**: the Moved and Slow benchamraks compile to be the same function.

### Clang x86:
Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
DoubleBenchmark::Orig * |       5 |        - | 182052188.7
DoubleBenchmark::Normal |       5 |    1.000 | 182788980.4
DoubleBenchmark::Moved |       5 |    1.000 | 182788980.4
DoubleBenchmark::Slow |       5 |    1.000 | 182788980.4

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
FloatBenchmark::Orig * |       5 |        - | 182788980.4
FloatBenchmark::Normal |       5 |    1.000 | 182052188.7
FloatBenchmark::Moved |       5 |    1.000 | 182052188.7
FloatBenchmark::Slow |       5 |    1.000 | 182788980.4

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
IntBenchmark::Orig * |       2 |        - | 421963612.5
IntBenchmark::Normal |       2 |    1.000 | 421963612.5
IntBenchmark::Moved |       2 |    1.000 | 421963612.5
IntBenchmark::Slow |       2 |    1.000 | 425943084.1

Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
LongBenchmark::Orig * |       2 |        - | 418057811.0
LongBenchmark::Normal |       2 |    1.000 | 418057811.0
LongBenchmark::Moved |       2 |    1.000 | 418057811.0
LongBenchmark::Slow |       2 |    1.000 | 418057811.0

Holy cow! Clang is fast. It actually optimized everything to be the same function.
```
Warning: DoubleBenchmark::Orig and DoubleBenchmark::Normal are benchmarks of the same function.
Warning: DoubleBenchmark::Orig and DoubleBenchmark::Moved are benchmarks of the same function.
Warning: DoubleBenchmark::Orig and DoubleBenchmark::Slow are benchmarks of the same function.
Warning: DoubleBenchmark::Normal and DoubleBenchmark::Moved are benchmarks of the same function.
Warning: DoubleBenchmark::Normal and DoubleBenchmark::Slow are benchmarks of the same function.
Warning: DoubleBenchmark::Moved and DoubleBenchmark::Slow are benchmarks of the same function.
Warning: FloatBenchmark::Orig and FloatBenchmark::Normal are benchmarks of the same function.
Warning: FloatBenchmark::Orig and FloatBenchmark::Moved are benchmarks of the same function.
Warning: FloatBenchmark::Orig and FloatBenchmark::Slow are benchmarks of the same function.
Warning: FloatBenchmark::Normal and FloatBenchmark::Moved are benchmarks of the same function.
Warning: FloatBenchmark::Normal and FloatBenchmark::Slow are benchmarks of the same function.
Warning: FloatBenchmark::Moved and FloatBenchmark::Slow are benchmarks of the same function.
Warning: IntBenchmark::Orig and IntBenchmark::Normal are benchmarks of the same function.
```
### Clang amd64
Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
DoubleBenchmark::Orig * |       5 |        - | 182788980.4
DoubleBenchmark::Normal |       5 |    1.000 | 182788980.4
DoubleBenchmark::Moved |       5 |    1.000 | 182788980.4
DoubleBenchmark::Slow |       5 |    1.000 | 182788980.4


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
FloatBenchmark::Orig * |       5 |        - | 182788980.4
FloatBenchmark::Normal |       5 |    1.000 | 182788980.4
FloatBenchmark::Moved |       5 |    1.000 | 182788980.4
FloatBenchmark::Slow |       5 |    1.000 | 182052188.7


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
IntBenchmark::Orig * |       2 |        - | 451506541.8
IntBenchmark::Normal |       2 |    1.000 | 451506541.8
IntBenchmark::Moved |       2 |    1.000 | 456065759.6
IntBenchmark::Slow |       2 |    1.000 | 456065759.6


Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
--------------------------|---------|----------|------------
LongBenchmark::Orig * |       2 |        - | 451506541.8
LongBenchmark::Normal |       2 |    1.000 | 447037577.3
LongBenchmark::Moved |       2 |    1.000 | 451506541.8
LongBenchmark::Slow |       2 |    1.000 | 456065759.6

Once again, fast times, and the same optimizations made everything act the same way.

## Conclusion

Overall, I feel since MSVC amd64, and Clang can achieve these fast times, there is an issue in MSVC x86, it appears to me compilers are more than powerful enough to optimize my claim that there will be an extra overhead of 32 bytes copied. Which I think makes sense, why would we use ```operator-``` if we were guaranteed a result that could be typically beaten by just writing a ```custom_minus(Obj& Out, Obj& LHS, Obj& RHS)```.