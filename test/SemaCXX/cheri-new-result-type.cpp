// RUN: %cheri_purecap_cc1 -fsyntax-only -ast-dump %s | FileCheck %s

class A { };

void f() {
  // CHECK: CXXNewExpr {{.*}} {{.*}} 'class A * __capability' Function
  A *a = new A;
}
