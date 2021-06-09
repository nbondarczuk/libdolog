#include "DoLog.hpp"

extern DoLog dl;

int main()
{
  DoLogAttValSet k;
  SQL_VALUE *v = new SQL_INTEGER("label", 1);
  k.Add(v);

  DoLogOp *op = dl.SqlOper(k, INSERT, "ANYTHING");
  SQL_VALUE *v1 = new SQL_INTEGER("labela", 10);
  SQL_VALUE *v2 = new SQL_INTEGER("labelb", 100);
  *op += v1;
  *op += v2;

  cout << dl.XmlUndo() << endl;

  return 0;
}
