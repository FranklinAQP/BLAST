import java.io.IOException;
import java.util.*;

import org.apache.hadoop.fs.Path; 
import org.apache.hadoop.conf.*; 
import org.apache.hadoop.io.*; 
import org.apache.hadoop.mapreduce.*;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;

public class Semillero{
	private static Text seq = new Text("AGTTTGTATGCTTAGTAGAGACGGGGTTTCACCGTTTTAGCTGGGATGGTCTCGATCTCC");
	public static class Map extends Mapper<LongWritable, Text, Text, IntWritable> {
		private final static IntWritable one = new IntWritable(1);
		private Text word = new Text();

		public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
			String line = value.toString();
			StringTokenizer tokenizer = new StringTokenizer(line);
			while (tokenizer.hasMoreTokens()) {
				word.set(tokenizer.nextToken());
				int i=0;
				Random randomGenerator = new Random();
				while (i<20){
					int randomInt = randomGenerator.nextInt(seq.getLength());
					if (randomInt < word.getLength()){
					   if (seq.charAt(randomInt)==word.charAt(randomInt)){
						context.write(word, one);
						//System.out.println("segundo if "+ i);
					   }	

					   ++i;
					}
					else if (word.getLength()<15) ++i;
				}
			}
		}
	}

	public static class Reduce extends Reducer<Text, IntWritable, Text, IntWritable> {
		public void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException {
			int sum = 0;
			for (IntWritable val : values ) 
				sum += val.get();
			
			if(sum > 7){
			   //System.out.println("reduce if "+ sum);
			   //context.write(key, new IntWritable(sum));
			   context.write(key,null);
			}
			
		}	
	}

	public static void main(String[] args) throws Exception {
		Configuration conf = new Configuration();
		
		Job job = new Job(conf, "Semillero");
		job.setJarByClass(Semillero.class);

		job.setOutputKeyClass(Text.class);
		job.setOutputValueClass(IntWritable.class);

		job.setMapperClass(Map.class);
		job.setReducerClass(Reduce.class);

		job.setInputFormatClass(TextInputFormat.class);
		job.setOutputFormatClass(TextOutputFormat.class);
	
		FileInputFormat.addInputPath(job, new Path(args[0]));
		FileOutputFormat.setOutputPath(job, new Path(args[1]));

		job.waitForCompletion(true);
	}
}
