; ModuleID = 'src/test_4.c'
source_filename = "src/test_4.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%struct.test = type { i32, [10 x i8], i8* }

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %t = alloca %struct.test*, align 8
  store i32 0, i32* %retval, align 4
  %call = call i8* @malloc(i64 24) #3
  %0 = bitcast i8* %call to %struct.test*
  store %struct.test* %0, %struct.test** %t, align 8
  %call1 = call i8* @malloc(i64 8) #3
  %1 = load %struct.test*, %struct.test** %t, align 8
  %str = getelementptr inbounds %struct.test, %struct.test* %1, i32 0, i32 2
  store i8* %call1, i8** %str, align 8
  %2 = load %struct.test*, %struct.test** %t, align 8
  %str2 = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 2
  %3 = load i8*, i8** %str2, align 8
  call void @free(i8* %3)
  %4 = load %struct.test*, %struct.test** %t, align 8
  %5 = bitcast %struct.test* %4 to i8*
  call void @free(i8* %5)
  ret i32 0
}

; Function Attrs: allocsize(0)
declare i8* @malloc(i64) #1

declare void @free(i8*) #2

attributes #0 = { noinline nounwind optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { allocsize(0) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { allocsize(0) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 8.0.0 (trunk 340813) (llvm/trunk 340812)"}
