package reflectPractice;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class ReflectPracticeMain {

    public String myName;
    public int no;
    
    public class TestMain{
        int  a;
        int  b;
    };
    public static void main(String[] args) {
        // TODO 自動生成されたメソッド・スタブ
        ReflectPracticeMain a = new ReflectPracticeMain();
        
        try {
            Field b = ReflectPracticeMain.class.getField("myName");
            b.set(a, "Test Name");
        } catch (NoSuchFieldException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        } catch (SecurityException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }
        a.no = 9;
        System.out.println(a.myName);
        System.out.println(String.format("%011d", a.no));
        

        List<TestMain> s = new ArrayList<TestMain>();
        TestMain m = a.new TestMain();
        m.a = 2;
        m.b = 3;
        s.add(m);
        
        TestMain n = a.new TestMain();
        n.a = 5;
        n.b = 6;
        s.add(n);
        
        System.out.println(s);
        
        if (TransformBetMapAndObj() instanceof Map) {
            System.out.println("Success");
        }
        Object mm = new Object();
        if (mm instanceof Map) {
            System.out.println("Success");
        }
    }
    
    public static Object TransformBetMapAndObj(){
        Map<String, String> a = new HashMap<String, String>();
        a.put("a", "b");
        return a;
    }

}
