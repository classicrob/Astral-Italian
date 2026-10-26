; ModuleID = '/Users/roberthaisfield/MacioGalaxy2-wt/gw543/native/tests/game_wait_codegen.cpp'
source_filename = "/Users/roberthaisfield/MacioGalaxy2-wt/gw543/native/tests/game_wait_codegen.cpp"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx27.0.0"

%struct.Exit = type { ptr, %"struct.PetariNative::GameWaitHistory::Token" }
%"struct.PetariNative::GameWaitHistory::Token" = type { i64, i64, i64, i64, i64, i64 }

@_ZN12PetariNative15GameWaitHistory5hooksE = linkonce_odr local_unnamed_addr global { { ptr } } zeroinitializer, align 8
@_ZN12PetariNative15GameWaitHistory7enabledE = linkonce_odr local_unnamed_addr global { { i8 } } zeroinitializer, align 1

; Function Attrs: mustprogress noinline ssp uwtable(sync)
define i32 @gw_probe_plain(ptr noundef %0) local_unnamed_addr #0 {
  %2 = tail call i32 @gw_work(ptr noundef %0)
  ret i32 %2
}

declare i32 @gw_work(ptr noundef) local_unnamed_addr #1

; Function Attrs: mustprogress noinline ssp uwtable(sync)
define i32 @gw_probe_guarded(ptr noundef %0) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
  %2 = alloca %struct.Exit, align 8
  %3 = load atomic i8, ptr @_ZN12PetariNative15GameWaitHistory7enabledE monotonic, align 1, !range !6, !noundef !7
  %4 = trunc nuw i8 %3 to i1
  br i1 %4, label %7, label %5

5:                                                ; preds = %1
  %6 = tail call i32 @gw_work(ptr noundef %0)
  br label %26

7:                                                ; preds = %1
  %8 = load atomic i64, ptr @_ZN12PetariNative15GameWaitHistory5hooksE acquire, align 8
  %9 = icmp eq i64 %8, 0
  br i1 %9, label %10, label %12

10:                                               ; preds = %7
  %11 = tail call i32 @gw_work(ptr noundef %0)
  br label %26

12:                                               ; preds = %7
  %13 = inttoptr i64 %8 to ptr
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %2) #3
  store ptr %13, ptr %2, align 8, !tbaa !8
  %14 = getelementptr inbounds nuw i8, ptr %2, i64 8
  %15 = load ptr, ptr %13, align 8, !tbaa !16
  call void %15(ptr dead_on_unwind nonnull writable sret(%"struct.PetariNative::GameWaitHistory::Token") align 8 %14, i64 noundef 14, i64 noundef 0, i64 noundef 0) #3
  %16 = invoke i32 @gw_work(ptr noundef %0)
          to label %17 unwind label %21

17:                                               ; preds = %12
  %18 = load ptr, ptr %2, align 8, !tbaa !8
  %19 = getelementptr inbounds nuw i8, ptr %18, i64 8
  %20 = load ptr, ptr %19, align 8, !tbaa !18
  call void %20(ptr noundef nonnull align 8 dereferenceable(48) %14) #3
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %2) #3
  br label %26

21:                                               ; preds = %12
  %22 = landingpad { ptr, i32 }
          cleanup
  %23 = load ptr, ptr %2, align 8, !tbaa !8
  %24 = getelementptr inbounds nuw i8, ptr %23, i64 8
  %25 = load ptr, ptr %24, align 8, !tbaa !18
  call void %25(ptr noundef nonnull align 8 dereferenceable(48) %14) #3
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %2) #3
  resume { ptr, i32 } %22

26:                                               ; preds = %5, %10, %17
  %27 = phi i32 [ %6, %5 ], [ %16, %17 ], [ %11, %10 ]
  ret i32 %27
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr captures(none)) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr captures(none)) #2

; Function Attrs: mustprogress noinline ssp uwtable(sync)
define void @gw_probe_metadata(ptr noundef %0) local_unnamed_addr #0 {
  %2 = load atomic i8, ptr @_ZN12PetariNative15GameWaitHistory7enabledE monotonic, align 1, !range !6, !noundef !7
  %3 = trunc nuw i8 %2 to i1
  br i1 %3, label %4, label %16

4:                                                ; preds = %1
  %5 = tail call i32 @gw_work(ptr noundef %0)
  %6 = sext i32 %5 to i64
  %7 = load atomic i8, ptr @_ZN12PetariNative15GameWaitHistory7enabledE monotonic, align 1, !range !6, !noundef !7
  %8 = trunc nuw i8 %7 to i1
  br i1 %8, label %9, label %16

9:                                                ; preds = %4
  %10 = load atomic i64, ptr @_ZN12PetariNative15GameWaitHistory5hooksE acquire, align 8
  %11 = icmp eq i64 %10, 0
  br i1 %11, label %16, label %12

12:                                               ; preds = %9
  %13 = inttoptr i64 %10 to ptr
  %14 = getelementptr inbounds nuw i8, ptr %13, i64 16
  %15 = load ptr, ptr %14, align 8, !tbaa !19
  tail call void %15(i64 noundef 12, i64 noundef %6, i64 noundef 0, i64 noundef 0) #3
  br label %16

16:                                               ; preds = %12, %9, %4, %1
  ret void
}

declare i32 @__gxx_personality_v0(...)

attributes #0 = { mustprogress noinline ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+bti,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+bti,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 27, i32 0]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Apple clang version 21.0.0 (clang-2100.3.34.2)"}
!6 = !{i8 0, i8 2}
!7 = !{}
!8 = !{!9, !10, i64 0}
!9 = !{!"_ZTSZN12PetariNative15GameWaitHistory6invokeIZ16gw_probe_guardedE3$_0EEDcNS0_4KindEyyOT_E4Exit", !10, i64 0, !14, i64 8}
!10 = !{!"p1 _ZTSN12PetariNative15GameWaitHistory5HooksE", !11, i64 0}
!11 = !{!"any pointer", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!"_ZTSN12PetariNative15GameWaitHistory5TokenE", !15, i64 0, !15, i64 8, !15, i64 16, !15, i64 24, !15, i64 32, !15, i64 40}
!15 = !{!"long long", !12, i64 0}
!16 = !{!17, !11, i64 0}
!17 = !{!"_ZTSN12PetariNative15GameWaitHistory5HooksE", !11, i64 0, !11, i64 8, !11, i64 16, !11, i64 24, !11, i64 32}
!18 = !{!17, !11, i64 8}
!19 = !{!17, !11, i64 16}
