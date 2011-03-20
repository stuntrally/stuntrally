#ifndef _CARTELEMETRY_H
#define _CARTELEMETRY_H

class CARTELEMETRY
{
	private:
		typedef double T;
	
		std::vector <std::pair <std::string, T> > variable_names;
		T time;
		bool wroteheader;
		const std::string telemetryname;
		std::ofstream file;
		
		void WriteHeader(const std::string & filename)
		{
			std::ofstream f((filename+".plt").c_str());
			assert(f);
			f << "plot ";
			unsigned int count = 0;
			for (std::vector <std::pair <std::string, T> >::iterator i =
				variable_names.begin(); i != variable_names.end(); ++i)
			{
				f << "\\" << std::endl << "\"" << filename+".dat" << "\" u 1:" << count+2 << " t '" << i->first << "' w lines";
				if (count < variable_names.size()-1)
					f << ",";
				f << " ";
				count++;
			}
			f << std::endl;
			wroteheader = true;
		}
		
	public:
		CARTELEMETRY(const std::string & name) : time(0), wroteheader(false), telemetryname(name), file((name+".dat").c_str()) {}
		CARTELEMETRY(const CARTELEMETRY & other) : variable_names(other.variable_names), time(other.time), wroteheader(other.wroteheader), telemetryname(other.telemetryname), file((telemetryname+".dat").c_str()) {}
		
		void AddRecord(const std::string & name, T value)
		{
			bool found = false;
			for (std::vector <std::pair <std::string, T> >::iterator i =
				variable_names.begin(); i != variable_names.end(); ++i)
			{
				if (name == i->first)
				{
					i->second = value;
					found = true;
					break;
				}
			}
			if (!found)
				variable_names.push_back(std::make_pair(name, value));
		}
		
		void Update(T dt)
		{
			if (time != 0 && !wroteheader)
				WriteHeader(telemetryname);
			time += dt;
			
			assert(file);
			file << time << " ";
			for (std::vector <std::pair <std::string, T> >::iterator i =
				variable_names.begin(); i != variable_names.end(); ++i)
				file << i->second << " ";
			file << "\n";
		}
};

#endif //_CARTELEMETRY_H
