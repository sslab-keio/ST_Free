; ModuleID = 'src/test.c'
source_filename = "src/test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, [10 x i8], %struct.ref* }
%struct.ref = type { i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %struct_parent = alloca %struct.test*, align 8
  store i32 0, i32* %retval, align 4
  %call = call noalias i8* @malloc(i64 24) #2
  %0 = bitcast i8* %call to %struct.test*
  store %struct.test* %0, %struct.test** %struct_parent, align 8
  %call1 = call noalias i8* @malloc(i64 4) #2
  %1 = bitcast i8* %call1 to %struct.ref*
  %2 = load %struct.test*, %struct.test** %struct_parent, align 8
  %b = getelementptr inbounds %struct.test, %struct.test* %2, i32 0, i32 2
  store %struct.ref* %1, %struct.ref** %b, align 8
  %3 = load %struct.test*, %struct.test** %struct_parent, align 8
  %4 = bitcast %struct.test* %3 to i8*
  call void @free(i8* %4) #2
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 "}
