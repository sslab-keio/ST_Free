; ModuleID = 'src/test_6.c'
source_filename = "src/test_6.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%struct.node = type { i32, %struct.node* }

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %head = alloca %struct.node*, align 8
  %next_node = alloca %struct.node*, align 8
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i8* @malloc(i64 16) #3
  %0 = bitcast i8* %call to %struct.node*
  store %struct.node* %0, %struct.node** %head, align 8
  %1 = load %struct.node*, %struct.node** %head, align 8
  %next = getelementptr inbounds %struct.node, %struct.node* %1, i32 0, i32 1
  store %struct.node* null, %struct.node** %next, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %2, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call1 = call i8* @malloc(i64 16) #3
  %3 = bitcast i8* %call1 to %struct.node*
  store %struct.node* %3, %struct.node** %next_node, align 8
  %4 = load %struct.node*, %struct.node** %head, align 8
  %next2 = getelementptr inbounds %struct.node, %struct.node* %4, i32 0, i32 1
  %5 = load %struct.node*, %struct.node** %next2, align 8
  %6 = load %struct.node*, %struct.node** %next_node, align 8
  %next3 = getelementptr inbounds %struct.node, %struct.node* %6, i32 0, i32 1
  store %struct.node* %5, %struct.node** %next3, align 8
  %7 = load %struct.node*, %struct.node** %next_node, align 8
  %8 = load %struct.node*, %struct.node** %head, align 8
  %next4 = getelementptr inbounds %struct.node, %struct.node* %8, i32 0, i32 1
  store %struct.node* %7, %struct.node** %next4, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, i32* %i, align 4
  %inc = add nsw i32 %9, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %10 = load %struct.node*, %struct.node** %head, align 8
  %11 = bitcast %struct.node* %10 to i8*
  call void @free(i8* %11)
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
!2 = !{!"clang version 8.0.0 (trunk 340517)"}
