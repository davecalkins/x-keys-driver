#include "stdafx.h"
#include "Utils.h"

void TokenizeString(LPCTSTR str, LPCTSTR sep, CStringArray& toks)
{
   ASSERT(str != NULL);
   if (str == NULL)
      return;

   ASSERT(sep != NULL);
   if (sep == NULL)
      return;

   CString _str(str);

   int curPos = 0;
   CString tok = _str.Tokenize(sep,curPos);
   while (tok != _T(""))
   {
      tok.Trim();
      if (tok != _T(""))
      {
         toks.Add(tok);
      }
      tok = _str.Tokenize(sep,curPos);
   }
}

void RemoveDuplicates(CStringArray& arr)
{
   CStringArray tmp;
   for (int i = 0; i < arr.GetSize(); i++)
   {
      BOOL AlreadyInTmp = FALSE;
      for (int j = 0; (j < tmp.GetSize()) && !AlreadyInTmp; j++)
      {
         if (arr[i].CompareNoCase(tmp[j]) == 0)
            AlreadyInTmp = TRUE;
      }
      if (!AlreadyInTmp)
         tmp.Add(arr[i]);
   }
   arr.RemoveAll();
   arr.Append(tmp);
}

int StringWordMatch(LPCTSTR strA, LPCTSTR strB)
{
   ASSERT(strA != NULL);
   if (strA == NULL)
      return 0;

   ASSERT(strB != NULL);
   if (strB == NULL)
      return 0;

   int score = 0;

   CStringArray toksA;
   TokenizeString(strA,_T(" "),toksA);
   RemoveDuplicates(toksA);

   CStringArray toksB;
   TokenizeString(strB,_T(" "),toksB);
   RemoveDuplicates(toksB);

   for (INT_PTR i = 0; i < toksA.GetSize(); i++)
   {
      for (INT_PTR j = 0; j < toksB.GetSize(); j++)
      {
         if (toksA[i].CompareNoCase(toksB[j]) == 0)
            score++;
      }
   }

   return score;
}

