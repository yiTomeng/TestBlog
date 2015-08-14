package com.yly.serial;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class SerializeTestMain implements  Serializable{

    public String name;
    public String address;
    public transient int SSN;
    public int number;
    public void mailCheck()
    {
       System.out.println("Mailing a check to " + name
                            + " " + address);
    }
    
    public class ObjStr {
        String j;
        String k;
    }
    public static void toUpperClassStr(String a) {
        a = a.toUpperCase();
        System.out.println("In toUpperClass");
        System.out.println(a);
        System.out.println("Out toUpperClass");
    }
    
    public static void toUpperClassStr(ObjStr a) {
        a.j = a.j.toUpperCase();
        System.out.println("In toUpperClass");
        System.out.println(a.j);
        System.out.println("Out toUpperClass");
    }
    
    public static void main(String[] args) {
        // TODO 自動生成されたメソッド・スタブ
        SerializeTestMain e = new SerializeTestMain();
        e.name = "Reyan Ali";
        e.address = "Phokka Kuan, Ambehta Peer";
        e.SSN = 11122333;
        e.number = 101;
        try
        {
           //OutputStream str = new OutputStream();
           FileOutputStream fileOut =
           new FileOutputStream("C:\\Users\\KOCHI\\Desktop\\ylyTest\\test2.txt");
           ObjectOutputStream out = new ObjectOutputStream(fileOut);
           out.writeObject(e);
           out.close();
           fileOut.close();
           System.out.println("Serialized data is saved in /tmp/employee.ser");
           /*
           List<String> a = new ArrayList<String>();
           String b = "2";
           String c = "3";
           a.add(b);
           a.add(c);
           int i = 0;
           for (String v : a) {
               System.out.println(v);
               if (i == 0) {
                   v = "b";
               }else{
                   v = "c";
               }
               System.out.println(v);
               i++;
           }
           
           for(String v: a) {
               System.out.println(v);
           }
           
           String m = "d";
           System.out.println(m);
           SerializeTestMain.toUpperClassStr(m);
           System.out.println(m);
           
           ObjStr n = new SerializeTestMain().new ObjStr();
           n.j = "f";
           SerializeTestMain.toUpperClassStr(n);
           System.out.println(n.j);
           */
           
           
           List<ObjStr> a = new ArrayList<ObjStr>();
           ObjStr b = new SerializeTestMain().new ObjStr();
           b.j = "2";
           ObjStr c = new SerializeTestMain().new ObjStr();
           c.j = "3";
           a.add(b);
           a.add(c);
           int i = 0;
           for (ObjStr v : a) {
               System.out.println(v.j);
               if (i == 0) {
                   v.j = "b";
               }else{
                   v.j = "c";
               }
               System.out.println(v.j);
               i++;
           }
           
           for(ObjStr v: a) {
               System.out.println(v.j);
           }
           //JSONArray a = new JSONArray();
        }catch(IOException i)
        {
            i.printStackTrace();
        }
    }

}
