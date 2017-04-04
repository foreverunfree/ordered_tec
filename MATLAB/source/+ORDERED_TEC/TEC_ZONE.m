classdef TEC_ZONE < ORDERED_TEC.TEC_ZONE_BASE
    %UNTITLED7 Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        Data;
        Echo_Mode; %zone_head, zone_end, variable, max_real, max_org, skip, begin & end, stdid & soltime, size
    end
    
    methods
        function obj = TEC_ZONE(varargin)
            if nargin==0
                obj.ZoneName = 'untitled_zone';
                obj.StrandId = -1;
                obj.SolutionTime = 0;
                obj.Skip = [1,1,1];
                obj.Begin = [1,1,1];
                obj.EEnd = [0,0,0];
                obj.Echo_Mode = 'brief';
            elseif nargin==1
                if isa(varargin{1},'numeric') && isequal(mod(varargin{1},1),zeros(size(varargin{1})))
                    obj = repmat(ORDERED_TEC.TEC_ZONE,varargin{1});
                else
                    ME = MException('TEC_ZONE:TypeWrong', 'constructor type wrong');
                    throw(ME);
                end
            else
                ME = MException('TEC_ZONE:NArgInWrong', 'too many input arguments');
                throw(ME);
            end
        end
        
        function obj = set.Echo_Mode(obj, zone_mode)
            if islogical(zone_mode) && ( isequal(size(zone_mode),[1,9]) || isequal(size(zone_mode),[9,1]) )
                obj.Echo_Mode = zone_mode;
            elseif ischar(zone_mode)
                if strcmp(zone_mode,'brief')
                    obj.Echo_Mode = logical([1,0,0,1,0,0,0,0,0]);
                elseif strcmp(zone_mode,'full')
                    obj.Echo_Mode = true(1,9);
                elseif strcmp(zone_mode,'simple')
                    obj.Echo_Mode = logical([1,0,0,0,0,0,0,0,0]);
                elseif strcmp(zone_mode,'none')
                    obj.Echo_Mode = false(1,9);
                elseif strcmp(zone_mode,'leave')
                else
                    ME = MException('TEC_ZONE:InputWrong', 'echo_mode code string wrong');
                    throw(ME);
                end
            else
                ME = MException('TEC_ZONE:TypeWrong', 'echo_mode type wrong');
                throw(ME);
            end
        end
        
    end
    
    methods (Hidden = true)
        function [obj,zone_log] = wrtie_plt_pre(obj,file,zone_log)
            if isempty(obj.Data)
                ME = MException('ORDERTEC:RuntimeError', ...
                    'file [%s]: zone [%s]: zone.Data is empty', file.FileName, obj.ZoneName);
                throw(ME);
            end
            if length(obj.Data)~=length(file.Variables)
                ME = MException('ORDERTEC:RuntimeError', ...
                    'file [%s]: zone [%s]: zone.Data is not correspond to tec_file.Variables', file.FileName, obj.ZoneName);
                throw(ME);
            end
            data_size = size(obj.Data{1});
            for kk = 1:length(obj.Data)
                da = obj.Data{kk};
                if ~isequal(size(da),data_size)
                    ME = MException('ORDERTEC:RuntimeError', ...
                        'file [%s]: zone [%s]: data size is not equal', file.FileName, obj.ZoneName);
                    throw(ME);
                end
                unvalid = isinf(da) | isnan(da);
                unvalid = any(unvalid(:));
                if unvalid
                    ME = MException('ORDERTEC:RuntimeError', ...
                        'file [%s]: zone [%s]: data is not valid (has nan or inf)', file.FileName, obj.ZoneName);
                    throw(ME);
                end
                [zone_log.Data(kk).type,zone_log.Data(kk).size_i] = gettype(da);
            end
            rijk = real_ijk(size(obj.Data{1}),obj.Skip,obj.Begin,obj.EEnd);
            if any(rijk <= 0)
                ME = MException('ORDERTEC:RuntimeError', ...
                    'file [%s]: zone [%s]: sum of Begin and End is not smaller than Max', file.FileName, obj.ZoneName);
                throw(ME);
            end
            zone_log.Max = real_ijk(size(obj.Data{1}),[1,1,1],[1,1,1],[0,0,0]);
            zone_log.Dim = find(zone_log.Max~=1,1,'last');
            zone_log.Real_Max = rijk;
            zone_log.Real_Dim = find(rijk~=1,1,'last');
            zone_log.noskip = isequal(obj.Skip,[1,1,1]);
            zone_log.noexc = isequal(obj.Begin,[1,1,1]) && isequal(obj.EEnd,[0,0,0]);
        end
        
        function write_plt_head(obj,fid)
            % iv   Zones
            fwrite(fid,299.0,'float32');% Zone marker. Value = 299.0
            fwrite(fid,ORDERED_TEC.s2i(obj.ZoneName),'int32');% Zone name
            fwrite(fid,-1,'int32');% ParentZone
            fwrite(fid,obj.StrandId,'int32');% StrandID
            fwrite(fid,obj.SolutionTime,'float64');% Solution time
            fwrite(fid,-1,'int32');% Not used. Set to -1
            fwrite(fid,0,'int32');% ZoneType 0=ORDERED
            fwrite(fid,0,'int32');% Specify Var Location. 0 = Don't specify, all data is located at the nodes
            fwrite(fid,0,'int32');% Are raw local 1-to-1 face neighbors supplied? (0=FALSE 1=TRUE) ORDERED and FELINESEG zones must specify 0
            fwrite(fid,0,'int32');% Number of miscellaneous user-defined face neighbor connections (value >= 0)
            fwrite(fid,real_ijk(size(obj.Data{1}),obj.Skip,obj.Begin,obj.EEnd),'int32');% IMax,JMax,KMax
            if ~isempty(obj.Auxiliary)
                for au = obj.Auxiliary
                    fwrite(fid,1,'int32');% Auxiliary name/value pair to follow
                    fwrite(fid,ORDERED_TEC.s2i(au{1}{1}),'int32');% name string
                    fwrite(fid,0,'int32');% Auxiliary Value Format (Currently only allow 0=AuxDataType_String)
                    fwrite(fid,ORDERED_TEC.s2i(au{1}{2}),'int32');% Value string
                end
            end
            fwrite(fid,0,'int32');% No more Auxiliary name/value pairs
        end
        
        function zone_log = write_plt_data(obj,fid,file,zone_log)
            pos_b = ftell(fid);
            fwrite(fid,299.0,'float32');% Zone marker. Value = 299.0
            val_n =0;
            for val = obj.Data
                val_n = val_n + 1;
                try
                    fwrite(fid,gettype(val{1}),'int32');% Variable data format
                catch t
                    ME = MException('OT:TypeError','zone [%s] var[%s]: class %s is not support',zone.ZoneName,tec_file.Variables{val_n},t.message);
                    throw(ME);
                end
            end
            fwrite(fid,0,'int32');% Has passive variables: 0 = no
            fwrite(fid,0,'int32');% Has variable sharing 0 = no
            fwrite(fid,-1,'int32');% Zero based zone number to share connectivity list with (-1 = no sharing)
            buf = cellfun(@(x)makebuf(x,obj.Skip,obj.Begin,obj.EEnd), obj.Data,'UniformOutput',false);
            for val = buf
                min_buf=min(val{1}(:));
                max_buf=max(val{1}(:));
                fwrite(fid,min_buf,'float64');% Min value
                fwrite(fid,max_buf,'float64');% Max value
            end
            
            if obj.Echo_Mode(4)
                echobuf = sprintf('     Dim = %i   Real_Max = [',zone_log.Real_Dim);
                for dd = 1:zone_log.Real_Dim
                    echobuf = [echobuf,' ',num2str(zone_log.Real_Max(dd))];
                end
                echobuf = [echobuf,' ]'];
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('%s\n',echobuf);
            end
            if obj.Echo_Mode(5)
                echobuf = sprintf('     Org_Dim = %i   Org_Max = [',zone_log.Dim);
                for dd = 1:zone_log.Dim
                    echobuf = [echobuf,' ',num2str(zone_log.Max(dd))];
                end
                echobuf = [echobuf,' ]'];
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('%s\n',echobuf);
            end
            if obj.Echo_Mode(6)
                echobuf = '     Skip = [';
                for dd = 1:3
                    echobuf = [echobuf,' ',num2str(zone_log.Skip(dd))];
                end
                echobuf = [echobuf,' ]'];
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('%s\n',echobuf);
            end
            if obj.Echo_Mode(7)
                echobuf = '     Begin = [';
                for dd = 1:3
                    echobuf = [echobuf,' ',num2str(zone_log.Begin(dd))];
                end
                echobuf = [echobuf,' ]'];
                echobuf = [echobuf,'   EEnd = ['];
                for dd = 1:3
                    echobuf = [echobuf,' ',num2str(zone_log.EEnd(dd))];
                end
                echobuf = [echobuf,' ]'];
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('%s\n',echobuf);
            end
            if obj.Echo_Mode(8) && obj.StrandId~=-1
                echobuf = sprintf('     StrandId = %i SolutionTime = %e',obj.StrandId,obj.SolutionTime);
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('%s\n',echobuf);
            end
            
            if obj.Echo_Mode(3)
                echobuf = '     write variables:';
                fprintf('%s',echobuf);
            end
            val_n =0;
            for val = buf
                val_n = val_n +1;
                v = val{1};
                if obj.Echo_Mode(3)
                    echobuf = [echobuf,' <',file.Variables{val_n},'>'];
                    fprintf(' <%s>',file.Variables{val_n});
                end
                zone_log.Data(val_n).file_pt = ftell(fid);
                fwrite(fid,v,class(v));% Zone Data. Each variable is in data format as specified above
            end
            if obj.Echo_Mode(3)
                e_l = length(zone_log.Echo_Text)+1;
                zone_log.Echo_Text{e_l} = echobuf;
                fprintf('\n');
            end
            
            pos_e = ftell(fid);
            zone_log.Size = (pos_e-pos_b)/1024/1024;
        end
        
    end
    
end

function rijk = real_ijk(ijk,skip,begin,eend)
% calculate real IJK from an array with its Skip, Begin and EEnd

if length(ijk)==2
    ijk = [ijk,1];
end
begin = begin-[1,1,1];
rijk=(ijk-begin-eend)./skip;
rijk=floor(rijk);
rijk(mod(ijk-begin-eend,skip)~=0)=rijk(mod(ijk-begin-eend,skip)~=0)+1;
end

function buf = makebuf(data,skip,begin,eend)
% make buffer data from an array with its Skip, Begin and EEnd

if isequal(skip,[1,1,1]) && isequal(begin,[1,1,1]) && isequal(eend,[0,0,0])
    buf = data;
else
    buf=data(begin(1):skip(1):end-eend(1), ...
        begin(2):skip(2):end-eend(2), ...
        begin(3):skip(3):end-eend(3));
end
if isempty(buf)
    ME = MException('ORDERTEC:ErrorVariable','one of Skip or Begin or EEnd is error');
    throw(ME);
end
end

function [ty,si] = gettype(data)
% get the data type (number) and its size

t=class(data);
switch t
    case 'single' %Float
        ty = 1;
        si = 4;
    case 'double' %Double
        ty = 2;
        si = 8;
    case 'int32'  %LongInt
        ty = 3;
        si = 4;
    case 'int16'  %ShortInt
        ty = 4;
        si = 2;
    case 'int8'   %Byte
        ty = 5;
        si = 1;
    otherwise
        ME = MException('OT:TypeError',t);
        throw(ME);
end
end
